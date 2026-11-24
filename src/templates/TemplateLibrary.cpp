#include "TemplateLibrary.h"
#include "core/ShowDocument.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariantMap>

namespace ps::templates {

QJsonObject Template::toJson() const
{
    QJsonObject o;
    if (!id.isEmpty()) o.insert(u"id", id);
    o.insert(u"name", name);
    if (!category.isEmpty()) o.insert(u"category", category);
    if (!color.isEmpty()) o.insert(u"color", color);
    if (isDefault) o.insert(u"isDefault", true);
    if (!settings.isEmpty()) o.insert(u"settings", settings);
    QJsonArray arr;
    for (const auto& item : items) arr.append(item.toJson());
    o.insert(u"items", arr);
    return o;
}

Template Template::fromJson(const QJsonObject& o)
{
    Template t;
    t.id = o.value(u"id").toString();
    t.name = o.value(u"name").toString();
    t.category = o.value(u"category").toString();
    t.color = o.value(u"color").toString();
    t.isDefault = o.value(u"isDefault").toBool(false);
    if (const auto v = o.constFind(u"settings"); v != o.constEnd() && v->isObject())
        t.settings = v->toObject();
    const auto arr = o.value(u"items").toArray();
    for (const auto& iv : arr) {
        if (iv.isObject()) t.items.push_back(Item::fromJson(iv.toObject()));
    }
    return t;
}

TemplateLibrary::TemplateLibrary(QObject* parent) : QObject(parent)
{
}

const Template* TemplateLibrary::find(const QString& id) const
{
    for (const auto& t : m_templates) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

void TemplateLibrary::rebuild()
{
    m_model.clear();
    for (const auto& t : m_templates) {
        QVariantMap m;
        m.insert(u"id", t.id);
        m.insert(u"name", t.name);
        m.insert(u"category", t.category);
        m.insert(u"color", t.color);
        m.insert(u"items", int(t.items.size()));
        m_model.append(m);
    }
    emit templatesChanged();
}

void TemplateLibrary::load()
{
    m_templates.clear();
    QFile file(m_filePath);
    if (file.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray arr;
        if (doc.isArray()) arr = doc.array();
        else if (doc.isObject()) arr = doc.object().value(u"templates").toArray();

        for (const auto& v : arr) {
            if (!v.isObject()) continue;
            auto t = Template::fromJson(v.toObject());
            if (t.id.isEmpty()) t.id = makeId();
            m_templates.append(std::move(t));
        }
    } else if (!m_filePath.isEmpty()) {
        emit errorOccurred(file.errorString());
    }
    rebuild();
}

bool TemplateLibrary::applyToSlide(ShowDocument& doc, const QString& slideId,
                                   const QString& templateId)
{
    const Template* t = find(templateId);
    if (!t) return false;
    const auto it = doc.show().slides.find(slideId);
    if (it == doc.show().slides.end()) return false;

    Slide updated = it->second;
    if (!t->items.empty()) {
        // Preserve the slide's text but adopt the template's styling/layout.
        const auto templateItem = t->items.front();
        for (auto& item : updated.items) {
            item.style = templateItem.style;
            item.align = templateItem.align;
            item.textFit = templateItem.textFit;
            item.autoFontSize = templateItem.autoFontSize;
        }
    }
    const auto bg = t->settings.value(u"backgroundPath").toString();
    if (!bg.isEmpty()) updated.settings.backgroundImage = bg;
    const auto color = t->settings.value(u"backgroundColor").toString();
    if (!color.isEmpty()) updated.settings.color = color;
    return doc.updateSlide(slideId, std::move(updated));
}

QString TemplateLibrary::createFromSlide(ShowDocument& doc, const QString& name,
                                         const QString& slideId)
{
    const auto it = doc.show().slides.constFind(slideId);
    if (it == doc.show().slides.constEnd()) return {};

    Template t;
    t.id = makeId();
    t.name = name.isEmpty() ? u"Template" : name;
    t.items = it->second.items;
    t.settings.insert(u"backgroundColor", it->second.settings.color.value_or(QString{}));
    if (it->second.settings.backgroundImage)
        t.settings.insert(u"backgroundPath", *it->second.settings.backgroundImage);
    m_templates.append(std::move(t));
    rebuild();
    return m_templates.last().id;
}

} // namespace ps::templates
