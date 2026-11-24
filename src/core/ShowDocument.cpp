#include "ShowDocument.h"

#include <algorithm>
#include <iterator>

namespace ps {

ShowDocument::ShowDocument(QObject* parent) : QObject(parent)
{
    if (m_show.id.isEmpty()) m_show.id = makeId();
    if (m_show.settings.activeLayout.isEmpty()) {
        const Id layoutId = makeId();
        Layout layout;
        layout.id = layoutId;
        layout.name = u"Main";
        m_show.layouts.insert(layoutId, layout);
        m_show.settings.activeLayout = layoutId;
    }
    m_show.timestamps.created = nowMillis();
}

void ShowDocument::setShow(Show show, bool clearHistory)
{
    m_show = std::move(show);
    if (clearHistory) {
        m_undo.clear();
        m_redo.clear();
        emit historyChanged();
    }
    m_modified = false;
    emit modifiedChanged(false);
    emit changed();
}

void ShowDocument::setName(const QString& name)
{
    if (m_show.name == name) return;
    pushUndo();
    m_show.name = name;
    commit();
}

void ShowDocument::pushUndo()
{
    m_undo.append(m_show.toJson());
    if (m_undo.size() > kMaxHistory) m_undo.removeFirst();
    m_redo.clear();
    emit historyChanged();
}

void ShowDocument::commit(const QString&)
{
    m_show.timestamps.modified = nowMillis();
    m_modified = true;
    emit modifiedChanged(true);
    emit changed();
}

bool ShowDocument::undo()
{
    if (m_undo.isEmpty()) return false;
    m_redo.append(m_show.toJson());
    m_show = Show::fromJson(m_undo.takeLast());
    m_modified = true;
    emit modifiedChanged(true);
    emit historyChanged();
    emit changed();
    return true;
}

bool ShowDocument::redo()
{
    if (m_redo.isEmpty()) return false;
    m_undo.append(m_show.toJson());
    m_show = Show::fromJson(m_redo.takeLast());
    m_modified = true;
    emit modifiedChanged(true);
    emit historyChanged();
    emit changed();
    return true;
}

void ShowDocument::markSaved()
{
    m_modified = false;
    emit modifiedChanged(false);
}

// ---------------------------------------------------------------------------
// Slides
// ---------------------------------------------------------------------------
Id ShowDocument::addSlide(Slide slide, const QString& group)
{
    pushUndo();
    if (slide.id.isEmpty()) slide.id = makeId();
    if (!group.isEmpty()) slide.group = group;
    m_show.slides[slide.id] = std::move(slide);
    appendToLayout(slide.id);
    commit();
    emit slideAdded(slide.id);
    return slide.id;
}

bool ShowDocument::updateSlide(const Id& id, Slide slide)
{
    const auto it = m_show.slides.find(id);
    if (it == m_show.slides.end()) return false;
    pushUndo();
    slide.id = id;
    it->second = std::move(slide);
    commit();
    return true;
}

bool ShowDocument::removeSlide(const Id& id)
{
    if (!m_show.slides.contains(id)) return false;
    pushUndo();
    m_show.slides.erase(id);
    if (auto lit = m_show.layouts.find(m_show.settings.activeLayout); lit != m_show.layouts.end()) {
        auto& slides = lit->second.slides;
        slides.erase(std::remove_if(slides.begin(), slides.end(),
                                    [&id](const SlideData& s) { return s.id == id; }),
                     slides.end());
    }
    commit();
    emit slideRemoved(id);
    return true;
}

Id ShowDocument::duplicateSlide(const Id& id)
{
    const auto it = m_show.slides.find(id);
    if (it == m_show.slides.end()) return {};
    pushUndo();
    Slide copy = it->second;
    copy.id = makeId();
    m_show.slides[copy.id] = copy;

    // insert right after the original in the active layout
    if (auto lit = m_show.layouts.find(m_show.settings.activeLayout); lit != m_show.layouts.end()) {
        auto& slides = lit->second.slides;
        auto pos = std::find_if(slides.begin(), slides.end(),
                                [&id](const SlideData& s) { return s.id == id; });
        SlideData ref;
        ref.id = copy.id;
        slides.insert(pos == slides.end() ? slides.end() : std::next(pos), std::move(ref));
    }
    commit();
    emit slideAdded(copy.id);
    return copy.id;
}

bool ShowDocument::moveSlide(const Id& id, int delta)
{
    auto lit = m_show.layouts.find(m_show.settings.activeLayout);
    if (lit == m_show.layouts.end() || delta == 0) return false;
    auto& slides = lit->second.slides;
    auto pos = std::find_if(slides.begin(), slides.end(),
                            [&id](const SlideData& s) { return s.id == id; });
    if (pos == slides.end()) return false;
    const auto target = std::clamp(int(std::distance(slides.begin(), pos)) + delta,
                                   0, int(slides.size()) - 1);
    const auto newPos = slides.begin() + target;
    if (newPos == pos) return false;
    pushUndo();
    std::iter_swap(pos, newPos);
    commit();
    return true;
}

Id ShowDocument::addLyricSlide(const QString& group, const QString& text,
                               const QString& key, const QString& style)
{
    Slide slide;
    slide.group = group.isEmpty() ? std::optional<QString>{} : std::optional<QString>(group);

    Item item;
    item.type = ItemType::Text;
    item.align = u"center";
    item.style = style.isEmpty()
        ? u"font-size: 52px; font-weight: 700; text-align: center; color: #f5f5f5;"
        : style;

    // Parse optional chord markers: "[C]When all [G]see is the battle"
    for (const auto& rawLine : splitSections(text)) {
        Line line;
        line.align = u"center";
        QString clean;
        int i = 0;
        while (i < rawLine.size()) {
            if (rawLine[i] == u'[') {
                const int close = rawLine.indexOf(u']', i + 1);
                if (close > i) {
                    Chord chord;
                    chord.id = makeId();
                    chord.pos = clean.size();
                    chord.key = rawLine.mid(i + 1, close - i - 1);
                    line.chords.push_back(std::move(chord));
                    i = close + 1;
                    continue;
                }
            }
            clean += rawLine[i++];
        }
        TextPart part;
        part.value = clean;
        part.style = u"color: #f5f5f5;";
        line.text.push_back(std::move(part));
        slide.items.push_back(Item{});
        slide.items.back() = item;
        slide.items.back().lines.push_back(std::move(line));
    }
    if (!key.isEmpty()) slide.key = key;
    return addSlide(std::move(slide), group);
}

Id ShowDocument::addMediaSlide(const QString& group, const QString& mediaPath, MediaType type)
{
    Slide slide;
    slide.group = group;
    Item item;
    item.type = (type == MediaType::Video) ? ItemType::Video : ItemType::Image;
    item.src = mediaPath;
    item.fit = u"contain";
    slide.items.push_back(std::move(item));
    return addSlide(std::move(slide), group);
}

// ---------------------------------------------------------------------------
// Layouts
// ---------------------------------------------------------------------------
Id ShowDocument::addLayout(const QString& name)
{
    pushUndo();
    Layout layout;
    layout.id = makeId();
    layout.name = name.isEmpty() ? u"Layout" : name;
    m_show.layouts.insert(layout.id, std::move(layout));
    commit();
    return layout.id;
}

bool ShowDocument::renameLayout(const Id& id, const QString& name)
{
    auto it = m_show.layouts.find(id);
    if (it == m_show.layouts.end()) return false;
    pushUndo();
    it->second.name = name;
    commit();
    return true;
}

bool ShowDocument::removeLayout(const Id& id)
{
    if (!m_show.layouts.contains(id) || m_show.layouts.size() == 1) return false;
    pushUndo();
    m_show.layouts.erase(id);
    if (m_show.settings.activeLayout == id)
        m_show.settings.activeLayout = m_show.layouts.begin()->first;
    commit();
    emit activeLayoutChanged(m_show.settings.activeLayout);
    return true;
}

bool ShowDocument::setActiveLayout(const Id& id)
{
    if (!m_show.layouts.contains(id) || m_show.settings.activeLayout == id) return false;
    pushUndo();
    m_show.settings.activeLayout = id;
    commit();
    emit activeLayoutChanged(id);
    return true;
}

bool ShowDocument::appendToLayout(const Id& slideId, const Id& layoutId)
{
    const Id target = layoutId.isEmpty() ? m_show.settings.activeLayout : layoutId;
    auto it = m_show.layouts.find(target);
    if (it == m_show.layouts.end()) return false;
    SlideData ref;
    ref.id = slideId;
    it->second.slides.push_back(std::move(ref));
    return true;
}

// ---------------------------------------------------------------------------
// Media
// ---------------------------------------------------------------------------
Id ShowDocument::addMedia(const MediaDescriptor& media)
{
    pushUndo();
    MediaDescriptor m = media;
    if (m.id.isEmpty()) m.id = makeId();
    m_show.media[m.id] = std::move(m);
    commit();
    return m.id;
}

bool ShowDocument::removeMedia(const Id& id)
{
    if (!m_show.media.contains(id)) return false;
    pushUndo();
    m_show.media.erase(id);
    commit();
    return true;
}

} // namespace ps
