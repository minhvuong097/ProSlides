#include "Types.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <QDateTime>

#include <algorithm>
#include <utility>

namespace ps {
namespace {

QString str(const QJsonObject& o, const char* key, const QString& fallback = {})
{
    const auto it = o.constFind(QLatin1String(key));
    if (it == o.constEnd() || !it->isString()) return fallback;
    return it->toString();
}

std::optional<QString> optStr(const QJsonObject& o, const char* key)
{
    const auto it = o.constFind(QLatin1String(key));
    if (it == o.constEnd() || it->isNull()) return std::nullopt;
    if (it->isString()) return it->toString();
    return std::nullopt;
}

bool boolean(const QJsonObject& o, const char* key, bool fallback = false)
{
    const auto it = o.constFind(QLatin1String(key));
    if (it == o.constEnd() || !it->isBool()) return fallback;
    return it->toBool();
}

double number(const QJsonObject& o, const char* key, double fallback = 0.0)
{
    const auto it = o.constFind(QLatin1String(key));
    if (it == o.constEnd() || !it->isDouble()) return fallback;
    return it->toDouble();
}

std::optional<int> optInt(const QJsonObject& o, const char* key)
{
    const auto it = o.constFind(QLatin1String(key));
    if (it == o.constEnd() || it->isNull() || !it->isDouble()) return std::nullopt;
    return static_cast<int>(it->toDouble());
}

std::optional<double> optNum(const QJsonObject& o, const char* key)
{
    const auto it = o.constFind(QLatin1String(key));
    if (it == o.constEnd() || it->isNull() || !it->isDouble()) return std::nullopt;
    return it->toDouble();
}

QStringList idList(const QJsonObject& o, const char* key)
{
    QStringList out;
    const auto it = o.constFind(QLatin1String(key));
    if (it == o.constEnd() || !it->isArray()) return out;
    for (const auto& v : it->toArray()) {
        if (v.isString()) out << v.toString();
    }
    return out;
}

std::vector<QString> toStringVector(const QStringList& l)
{
    return std::vector<QString>(l.begin(), l.end());
}

void putOpt(QJsonObject& o, const char* key, const std::optional<QString>& v)
{
    if (v) o.insert(QLatin1String(key), *v);
}
void putOptNum(QJsonObject& o, const char* key, const std::optional<int>& v)
{
    if (v) o.insert(QLatin1String(key), *v);
}
void putOptDouble(QJsonObject& o, const char* key, const std::optional<double>& v)
{
    if (v) o.insert(QLatin1String(key), *v);
}

} // namespace

// ---------------------------------------------------------------------------
// Enum string maps
// ---------------------------------------------------------------------------
QString itemTypeToString(ItemType t)
{
    switch (t) {
    case ItemType::Text: return "text";
    case ItemType::Media: return "media";
    case ItemType::Image: return "image";
    case ItemType::Video: return "video";
    case ItemType::Audio: return "audio";
    case ItemType::Camera: return "camera";
    case ItemType::Timer: return "timer";
    case ItemType::Clock: return "clock";
    case ItemType::Button: return "button";
    case ItemType::Events: return "events";
    case ItemType::Weather: return "weather";
    case ItemType::Variable: return "variable";
    case ItemType::Web: return "web";
    case ItemType::QrCode: return "qr_code";
    case ItemType::Icon: return "icon";
    case ItemType::SlideTracker: return "slide_tracker";
    case ItemType::Visualizer: return "visualizer";
    case ItemType::Captions: return "captions";
    case ItemType::Metronome: return "metronome";
    case ItemType::CurrentOutput: return "current_output";
    case ItemType::Chart: return "chart";
    case ItemType::Table: return "table";
    case ItemType::Effect: return "effect";
    case ItemType::Screen: return "screen";
    case ItemType::Ndi: return "ndi";
    case ItemType::Player: return "player";
    case ItemType::Overlay: return "overlay";
    case ItemType::Pdf: return "pdf";
    case ItemType::Ppt: return "ppt";
    case ItemType::Folder: return "folder";
    case ItemType::Divider: return "divider";
    case ItemType::Show: return "show";
    case ItemType::Section: return "section";
    case ItemType::Placeholder: return "show_placeholder";
    case ItemType::Unknown: break;
    }
    return {};
}

ItemType itemTypeFromString(const QString& s)
{
    static const std::unordered_map<QString, ItemType> map = {
        {u"text", ItemType::Text}, {u"media", ItemType::Media},
        {u"image", ItemType::Image}, {u"video", ItemType::Video},
        {u"audio", ItemType::Audio}, {u"camera", ItemType::Camera},
        {u"timer", ItemType::Timer}, {u"clock", ItemType::Clock},
        {u"button", ItemType::Button}, {u"events", ItemType::Events},
        {u"weather", ItemType::Weather}, {u"variable", ItemType::Variable},
        {u"web", ItemType::Web}, {u"qr_code", ItemType::QrCode},
        {u"icon", ItemType::Icon}, {u"slide_tracker", ItemType::SlideTracker},
        {u"visualizer", ItemType::Visualizer}, {u"captions", ItemType::Captions},
        {u"metronome", ItemType::Metronome}, {u"current_output", ItemType::CurrentOutput},
        {u"chart", ItemType::Chart}, {u"table", ItemType::Table},
        {u"effect", ItemType::Effect}, {u"screen", ItemType::Screen},
        {u"ndi", ItemType::Ndi}, {u"player", ItemType::Player},
        {u"overlay", ItemType::Overlay}, {u"pdf", ItemType::Pdf},
        {u"ppt", ItemType::Ppt}, {u"folder", ItemType::Folder},
        {u"divider", ItemType::Divider}, {u"show", ItemType::Show},
        {u"section", ItemType::Section}, {u"show_placeholder", ItemType::Placeholder}
    };
    const auto it = map.find(s);
    return it == map.end() ? ItemType::Unknown : it->second;
}

QString showTypeToString(ShowType t)
{
    switch (t) {
    case ShowType::Divider: return "DIVIDER";
    case ShowType::Show: return "show";
    case ShowType::Image: return "image";
    case ShowType::Video: return "video";
    case ShowType::Audio: return "audio";
    case ShowType::Player: return "player";
    case ShowType::Section: return "section";
    case ShowType::Overlay: return "overlay";
    case ShowType::Effect: return "effect";
    case ShowType::Pdf: return "pdf";
    case ShowType::Ppt: return "ppt";
    case ShowType::Screen: return "screen";
    case ShowType::Ndi: return "ndi";
    case ShowType::Camera: return "camera";
    case ShowType::Folder: return "folder";
    case ShowType::ShowPlaceholder: return "show_placeholder";
    }
    return {};
}

ShowType showTypeFromString(const QString& s)
{
    static const std::unordered_map<QString, ShowType> map = {
        {u"DIVIDER", ShowType::Divider}, {u"show", ShowType::Show},
        {u"image", ShowType::Image}, {u"video", ShowType::Video},
        {u"audio", ShowType::Audio}, {u"player", ShowType::Player},
        {u"section", ShowType::Section}, {u"overlay", ShowType::Overlay},
        {u"effect", ShowType::Effect}, {u"pdf", ShowType::Pdf},
        {u"ppt", ShowType::Ppt}, {u"screen", ShowType::Screen},
        {u"ndi", ShowType::Ndi}, {u"camera", ShowType::Camera},
        {u"folder", ShowType::Folder}, {u"show_placeholder", ShowType::ShowPlaceholder}
    };
    const auto it = map.find(s);
    return it == map.end() ? ShowType::Show : it->second;
}

QString transitionTypeToString(TransitionType t)
{
    switch (t) {
    case TransitionType::None: return "none";
    case TransitionType::Blur: return "blur";
    case TransitionType::Fade: return "fade";
    case TransitionType::Crossfade: return "crossfade";
    case TransitionType::Fly: return "fly";
    case TransitionType::Scale: return "scale";
    case TransitionType::Slide: return "slide";
    case TransitionType::Spin: return "spin";
    }
    return "none";
}

TransitionType transitionTypeFromString(const QString& s)
{
    static const std::unordered_map<QString, TransitionType> map = {
        {u"none", TransitionType::None}, {u"blur", TransitionType::Blur},
        {u"fade", TransitionType::Fade}, {u"crossfade", TransitionType::Crossfade},
        {u"fly", TransitionType::Fly}, {u"scale", TransitionType::Scale},
        {u"slide", TransitionType::Slide}, {u"spin", TransitionType::Spin}
    };
    const auto it = map.find(s);
    return it == map.end() ? TransitionType::None : it->second;
}

QString mediaTypeToString(MediaType t)
{
    switch (t) {
    case MediaType::Media: return "media";
    case MediaType::Video: return "video";
    case MediaType::Image: return "image";
    case MediaType::Effect: return "effect";
    case MediaType::Screen: return "screen";
    case MediaType::Ndi: return "ndi";
    case MediaType::Omt: return "omt";
    case MediaType::Camera: return "camera";
    case MediaType::Player: return "player";
    case MediaType::Audio: return "audio";
    }
    return "media";
}

MediaType mediaTypeFromString(const QString& s)
{
    static const std::unordered_map<QString, MediaType> map = {
        {u"media", MediaType::Media}, {u"video", MediaType::Video},
        {u"image", MediaType::Image}, {u"effect", MediaType::Effect},
        {u"screen", MediaType::Screen}, {u"ndi", MediaType::Ndi},
        {u"omt", MediaType::Omt}, {u"camera", MediaType::Camera},
        {u"player", MediaType::Player}, {u"audio", MediaType::Audio}
    };
    const auto it = map.find(s);
    return it == map.end() ? MediaType::Media : it->second;
}

// ---------------------------------------------------------------------------
// Resolution / Cropping / Chord / TextPart / Line / ListItem
// ---------------------------------------------------------------------------
QJsonObject Resolution::toJson() const
{
    QJsonObject o;
    o.insert(u"width", width);
    o.insert(u"height", height);
    return o;
}
Resolution Resolution::fromJson(const QJsonObject& o)
{
    Resolution r;
    if (const auto v = o.constFind(u"width"); v != o.constEnd() && v->isDouble()) r.width = int(v->toDouble());
    if (const auto v = o.constFind(u"height"); v != o.constEnd() && v->isDouble()) r.height = int(v->toDouble());
    return r;
}

QJsonObject Chord::toJson() const
{
    QJsonObject o;
    o.insert(u"id", id);
    o.insert(u"pos", pos);
    o.insert(u"key", key);
    return o;
}
static Chord chordFromJson(const QJsonObject& o)
{
    Chord c;
    c.id = str(o, "id");
    c.pos = number(o, "pos");
    c.key = str(o, "key");
    return c;
}

QJsonObject TextPart::toJson() const
{
    QJsonObject o;
    o.insert(u"value", value);
    o.insert(u"style", style);
    putOpt(o, "customType", customType);
    putOpt(o, "sourceDynamicKey", sourceDynamicKey);
    return o;
}
static TextPart textPartFromJson(const QJsonObject& o)
{
    TextPart p;
    p.value = str(o, "value");
    p.style = str(o, "style");
    p.customType = optStr(o, "customType");
    p.sourceDynamicKey = optStr(o, "sourceDynamicKey");
    return p;
}

QString Line::plainText() const
{
    QString out;
    for (const auto& p : text) out += p.value;
    return out;
}

QJsonObject Line::toJson() const
{
    QJsonObject o;
    o.insert(u"align", align);
    QJsonArray arr;
    for (const auto& p : text) arr.append(p.toJson());
    o.insert(u"text", arr);
    if (!chords.empty()) {
        QJsonArray ca;
        for (const auto& c : chords) ca.append(c.toJson());
        o.insert(u"chords", ca);
    }
    return o;
}
static Line lineFromJson(const QJsonObject& o)
{
    Line l;
    l.align = str(o, "align", "center");
    if (const auto it = o.constFind(u"text"); it != o.constEnd() && it->isArray()) {
        for (const auto& v : it->toArray()) {
            if (v.isObject()) l.text.push_back(textPartFromJson(v.toObject()));
        }
    }
    if (const auto it = o.constFind(u"chords"); it != o.constEnd() && it->isArray()) {
        for (const auto& v : it->toArray()) {
            if (v.isObject()) l.chords.push_back(chordFromJson(v.toObject()));
        }
    }
    return l;
}

QJsonObject ListItem::toJson() const
{
    QJsonObject o;
    o.insert(u"text", text);
    putOpt(o, "icon", icon);
    return o;
}

QJsonObject ListData::toJson() const
{
    QJsonObject o;
    if (enabled) o.insert(u"enabled", true);
    putOpt(o, "style", style);
    putOptNum(o, "interval", interval);
    if (!items.empty()) {
        QJsonArray arr;
        for (const auto& i : items) arr.append(i.toJson());
        o.insert(u"items", arr);
    }
    return o;
}

static ListData listFromJson(const QJsonObject& o)
{
    ListData l;
    l.enabled = boolean(o, "enabled");
    l.style = optStr(o, "style");
    l.interval = optInt(o, "interval");
    if (const auto it = o.constFind(u"items"); it != o.constEnd() && it->isArray()) {
        for (const auto& v : it->toArray()) {
            if (!v.isObject()) continue;
            const auto io = v.toObject();
            ListItem item;
            item.text = str(io, "text");
            item.icon = optStr(io, "icon");
            l.items.push_back(std::move(item));
        }
    }
    return l;
}

QJsonObject Scrolling::toJson() const
{
    QJsonObject o;
    o.insert(u"type", type);
    putOptDouble(o, "speed", speed);
    putOptDouble(o, "gap", gap);
    return o;
}
static Scrolling scrollingFromJson(const QJsonObject& o)
{
    Scrolling s;
    s.type = str(o, "type", "none");
    s.speed = optNum(o, "speed");
    s.gap = optNum(o, "gap");
    return s;
}

static Cropping croppingFromJson(const QJsonObject& o)
{
    Cropping c;
    c.top = number(o, "top"); c.right = number(o, "right");
    c.bottom = number(o, "bottom"); c.left = number(o, "left");
    c.type = str(o, "type");
    return c;
}
static QJsonObject croppingToJson(const Cropping& c)
{
    QJsonObject o;
    o.insert(u"top", c.top); o.insert(u"right", c.right);
    o.insert(u"bottom", c.bottom); o.insert(u"left", c.left);
    if (!c.type.isEmpty()) o.insert(u"type", c.type);
    return o;
}

// ---------------------------------------------------------------------------
// Item
// ---------------------------------------------------------------------------
QJsonObject Item::toJson() const
{
    QJsonObject o = extra;
    if (!id.isEmpty()) o.insert(u"id", id);
    o.insert(u"type", itemTypeToString(type));
    o.insert(u"style", style);
    putOpt(o, "align", align);
    if (!lines.empty()) {
        QJsonArray arr;
        for (const auto& l : lines) arr.append(l.toJson());
        o.insert(u"lines", arr);
    }
    if (list) o.insert(u"list", list->toJson());
    putOpt(o, "textFit", textFit);
    putOptNum(o, "autoFontSize", autoFontSize);
    putOpt(o, "src", src);
    putOpt(o, "customSvg", customSvg);
    putOpt(o, "filter", filter);
    putOpt(o, "blend", blend);
    if (cropping) o.insert(u"cropping", croppingToJson(*cropping));
    if (muted) o.insert(u"muted", true);
    if (loop) o.insert(u"loop", true);
    if (volume != 1.0) o.insert(u"volume", volume);
    if (speed != 1.0) o.insert(u"speed", speed);
    putOptDouble(o, "fromTime", fromTime);
    putOptDouble(o, "toTime", toTime);
    putOpt(o, "fit", fit);
    if (decoration) o.insert(u"decoration", true);
    if (clickReveal) o.insert(u"clickReveal", true);
    if (lineReveal) o.insert(u"lineReveal", true);
    if (!bindings.empty()) {
        QJsonArray arr;
        for (const auto& b : bindings) arr.append(b);
        o.insert(u"bindings", arr);
    }
    if (!actions.isEmpty()) o.insert(u"actions", actions);
    if (scrolling) o.insert(u"scrolling", scrolling->toJson());
    if (!chordsStyle.isEmpty()) o.insert(u"chords", chordsStyle);
    putOpt(o, "language", language);
    putOpt(o, "timerId", timerId);
    if (!timer.isEmpty()) o.insert(u"timer", timer);
    if (!clock.isEmpty()) o.insert(u"clock", clock);
    if (!events.isEmpty()) o.insert(u"events", events);
    if (!qrCode.isEmpty()) o.insert(u"qr_code", qrCode);
    if (!weather.isEmpty()) o.insert(u"weather", weather);
    if (!table.isEmpty()) o.insert(u"table", table);
    if (!chart.isEmpty()) o.insert(u"chart", chart);
    return o;
}

Item Item::fromJson(const QJsonObject& o)
{
    Item it;
    it.id = str(o, "id");
    it.type = itemTypeFromString(str(o, "type", "text"));
    it.style = str(o, "style");
    it.align = optStr(o, "align");
    if (const auto v = optStr(o, "specialStyle")) it.specialStyle = *v;
    if (const auto v = o.constFind(u"lines"); v != o.constEnd() && v->isArray()) {
        for (const auto& lv : v->toArray())
            if (lv.isObject()) it.lines.push_back(lineFromJson(lv.toObject()));
    }
    if (const auto v = o.constFind(u"list"); v != o.constEnd() && v->isObject())
        it.list = listFromJson(v->toObject());
    it.textFit = optStr(o, "textFit");
    it.autoFontSize = optInt(o, "autoFontSize");
    it.src = optStr(o, "src");
    it.customSvg = optStr(o, "customSvg");
    it.filter = optStr(o, "filter");
    it.blend = optStr(o, "blend");
    if (const auto v = o.constFind(u"cropping"); v != o.constEnd() && v->isObject())
        it.cropping = croppingFromJson(v->toObject());
    it.muted = boolean(o, "muted");
    it.loop = boolean(o, "loop");
    it.volume = number(o, "volume", 1.0);
    it.speed = number(o, "speed", 1.0);
    it.fromTime = optNum(o, "fromTime");
    it.toTime = optNum(o, "toTime");
    it.fit = optStr(o, "fit");
    it.decoration = boolean(o, "decoration");
    it.clickReveal = boolean(o, "clickReveal");
    it.lineReveal = boolean(o, "lineReveal");
    it.bindings = idList(o, "bindings");
    if (const auto v = o.constFind(u"actions"); v != o.constEnd() && v->isObject())
        it.actions = v->toObject();
    if (const auto v = o.constFind(u"scrolling"); v != o.constEnd() && v->isObject())
        it.scrolling = scrollingFromJson(v->toObject());
    if (const auto v = o.constFind(u"chords"); v != o.constEnd() && v->isObject())
        it.chordsStyle = v->toObject();
    it.language = optStr(o, "language");
    it.timerId = optStr(o, "timerId");
    if (const auto v = o.constFind(u"timer"); v != o.constEnd() && v->isObject()) it.timer = v->toObject();
    if (const auto v = o.constFind(u"clock"); v != o.constEnd() && v->isObject()) it.clock = v->toObject();
    if (const auto v = o.constFind(u"events"); v != o.constEnd() && v->isObject()) it.events = v->toObject();
    if (const auto v = o.constFind(u"qr_code"); v != o.constEnd() && v->isObject()) it.qrCode = v->toObject();
    if (const auto v = o.constFind(u"weather"); v != o.constEnd() && v->isObject()) it.weather = v->toObject();
    if (const auto v = o.constFind(u"table"); v != o.constEnd() && v->isObject()) it.table = v->toObject();
    if (const auto v = o.constFind(u"chart"); v != o.constEnd() && v->isObject()) it.chart = v->toObject();

    // Preserve unrecognised fields for round-trip compatibility.
    static const QStringList known = {
        u"id", u"type", u"style", u"align", u"specialStyle", u"lines", u"list",
        u"textFit", u"autoFontSize", u"src", u"customSvg", u"filter", u"blend",
        u"cropping", u"muted", u"loop", u"volume", u"speed", u"fromTime",
        u"toTime", u"fit", u"decoration", u"clickReveal", u"lineReveal",
        u"bindings", u"actions", u"scrolling", u"chords", u"language",
        u"timerId", u"timer", u"clock", u"events", u"qr_code", u"weather",
        u"table", u"chart", u"device", u"web", u"tracker", u"variable",
        u"button", u"visualizer", u"captions", u"conditions", u"fromTemplate"
    };
    for (auto i = o.constBegin(); i != o.constEnd(); ++i) {
        if (!known.contains(i.key())) it.extra.insert(i.key(), i.value());
    }
    return it;
}

QString Item::plainText() const
{
    QStringList parts;
    for (const auto& l : lines) parts << l.plainText();
    return parts.join(u'\n');
}

// ---------------------------------------------------------------------------
// MediaDescriptor
// ---------------------------------------------------------------------------
QJsonObject MediaDescriptor::toJson() const
{
    QJsonObject o;
    if (!id.isEmpty()) o.insert(u"id", id);
    o.insert(u"name", name);
    o.insert(u"path", path);
    o.insert(u"type", mediaTypeToString(type));
    putOpt(o, "videoType", videoType);
    putOpt(o, "cameraGroup", cameraGroup);
    if (muted) o.insert(u"muted", true);
    if (loop) o.insert(u"loop", true);
    putOpt(o, "filters", filters);
    putOpt(o, "base64", base64);
    if (!cloud.isEmpty()) o.insert(u"cloud", cloud);
    return o;
}
MediaDescriptor MediaDescriptor::fromJson(const QJsonObject& o)
{
    MediaDescriptor m;
    m.id = str(o, "id");
    m.name = str(o, "name");
    m.path = str(o, "path");
    m.type = mediaTypeFromString(str(o, "type", "media"));
    m.videoType = optStr(o, "videoType");
    m.cameraGroup = optStr(o, "cameraGroup");
    m.muted = boolean(o, "muted");
    m.loop = boolean(o, "loop");
    m.filters = optStr(o, "filters");
    m.base64 = optStr(o, "base64");
    if (const auto v = o.constFind(u"cloud"); v != o.constEnd() && v->isObject()) m.cloud = v->toObject();
    return m;
}

// ---------------------------------------------------------------------------
// Transition
// ---------------------------------------------------------------------------
static TransitionData transitionDataFromJson(const QJsonObject& o)
{
    TransitionData d;
    d.type = transitionTypeFromString(str(o, "type"));
    d.duration = number(o, "duration");
    d.easing = str(o, "easing", "linear");
    d.delay = optNum(o, "delay");
    return d;
}
static QJsonObject transitionDataToJson(const TransitionData& d)
{
    QJsonObject o;
    o.insert(u"type", transitionTypeToString(d.type));
    o.insert(u"duration", d.duration);
    o.insert(u"easing", d.easing);
    putOptDouble(o, "delay", d.delay);
    return o;
}

QJsonObject Transition::toJson() const
{
    QJsonObject o;
    o.insert(u"type", transitionTypeToString(type));
    o.insert(u"duration", duration);
    o.insert(u"easing", easing);
    if (in) o.insert(u"in", transitionDataToJson(*in));
    if (out) o.insert(u"out", transitionDataToJson(*out));
    if (between) o.insert(u"between", transitionDataToJson(*between));
    putOpt(o, "custom", custom);
    return o;
}
Transition Transition::fromJson(const QJsonValue& v)
{
    Transition t;
    if (v.isString()) {
        t.type = transitionTypeFromString(v.toString());
    } else if (v.isObject()) {
        const auto o = v.toObject();
        t.type = transitionTypeFromString(str(o, "type"));
        t.duration = number(o, "duration");
        t.easing = str(o, "easing", "linear");
        if (const auto x = o.constFind(u"in"); x != o.constEnd() && x->isObject())
            t.in = transitionDataFromJson(x->toObject());
        if (const auto x = o.constFind(u"out"); x != o.constEnd() && x->isObject())
            t.out = transitionDataFromJson(x->toObject());
        if (const auto x = o.constFind(u"between"); x != o.constEnd() && x->isObject())
            t.between = transitionDataFromJson(x->toObject());
        t.custom = optStr(o, "custom");
    }
    return t;
}

// ---------------------------------------------------------------------------
// SlideAction / SlideData / Layout
// ---------------------------------------------------------------------------
QJsonObject SlideAction::toJson() const
{
    QJsonObject o;
    o.insert(u"id", id);
    o.insert(u"name", name);
    QJsonArray tr;
    for (const auto& t : triggers) tr.append(t);
    o.insert(u"triggers", tr);
    if (!actionValues.isEmpty()) o.insert(u"actionValues", actionValues);
    if (!customData.isEmpty()) o.insert(u"customData", customData);
    return o;
}
static SlideAction slideActionFromJson(const QJsonObject& o)
{
    SlideAction a;
    a.id = str(o, "id");
    a.name = str(o, "name");
    a.triggers = idList(o, "triggers");
    if (const auto v = o.constFind(u"actionValues"); v != o.constEnd() && v->isObject())
        a.actionValues = v->toObject();
    if (const auto v = o.constFind(u"customData"); v != o.constEnd() && v->isObject())
        a.customData = v->toObject();
    return a;
}

QJsonObject SlideData::toJson() const
{
    QJsonObject o;
    o.insert(u"id", id);
    if (disabled) o.insert(u"disabled", true);
    putOpt(o, "parent", parent);
    if (!children.empty()) {
        QJsonArray a;
        for (const auto& c : children) a.append(c);
        o.insert(u"children", a);
    }
    putOpt(o, "color", color);
    putOptNum(o, "nextTimer", nextTimer);
    if (transition) o.insert(u"transition", transition->toJson());
    putOpt(o, "filter", filter);
    if (end) o.insert(u"end", true);
    putOptNum(o, "timer", timer);
    putOpt(o, "background", background);
    auto appendIds = [&o](const char* key, const std::vector<Id>& ids) {
        if (ids.empty()) return;
        QJsonArray a;
        for (const auto& x : ids) a.append(x);
        o.insert(QLatin1String(key), a);
    };
    appendIds("overlays", overlays);
    appendIds("effects", effects);
    appendIds("audio", audio);
    if (remove) o.insert(u"remove", true);
    if (!actions.empty()) {
        QJsonArray a;
        for (const auto& act : actions) a.append(act.toJson());
        o.insert(u"actions", a);
    }
    if (!bindings.empty()) {
        QJsonArray a;
        for (const auto& b : bindings) a.append(b);
        o.insert(u"bindings", a);
    }
    putOptNum(o, "breakDuration", breakDuration);
    return o;
}
SlideData SlideData::fromJson(const QJsonObject& o)
{
    SlideData s;
    s.id = str(o, "id");
    s.disabled = boolean(o, "disabled");
    s.parent = optStr(o, "parent");
    s.children = toStringVector(idList(o, "children"));
    s.color = optStr(o, "color");
    s.nextTimer = optInt(o, "nextTimer");
    if (const auto v = o.constFind(u"transition"); v != o.constEnd())
        s.transition = Transition::fromJson(*v);
    s.filter = optStr(o, "filter");
    s.end = boolean(o, "end");
    s.timer = optInt(o, "timer");
    s.background = optStr(o, "background");
    s.overlays = toStringVector(idList(o, "overlays"));
    s.effects = toStringVector(idList(o, "effects"));
    s.audio = toStringVector(idList(o, "audio"));
    s.remove = boolean(o, "remove");
    if (const auto v = o.constFind(u"actions"); v != o.constEnd() && v->isArray()) {
        for (const auto& av : v->toArray())
            if (av.isObject()) s.actions.push_back(slideActionFromJson(av.toObject()));
    }
    s.bindings = toStringVector(idList(o, "bindings"));
    s.breakDuration = optInt(o, "breakDuration");
    return s;
}

QJsonObject Layout::toJson() const
{
    QJsonObject o;
    if (!id.isEmpty()) o.insert(u"id", id);
    o.insert(u"name", name);
    o.insert(u"notes", notes);
    if (timeline) o.insert(u"timeline", *timeline);
    QJsonArray a;
    for (const auto& s : slides) a.append(s.toJson());
    o.insert(u"slides", a);
    return o;
}
Layout Layout::fromJson(const QJsonObject& o)
{
    Layout l;
    l.id = str(o, "id");
    l.name = str(o, "name");
    l.notes = str(o, "notes");
    if (const auto v = o.constFind(u"timeline"); v != o.constEnd() && v->isObject())
        l.timeline = v->toObject();
    if (const auto v = o.constFind(u"slides"); v != o.constEnd() && v->isArray()) {
        for (const auto& sv : v->toArray())
            if (sv.isObject()) l.slides.push_back(SlideData::fromJson(sv.toObject()));
    }
    return l;
}

// ---------------------------------------------------------------------------
// Slide
// ---------------------------------------------------------------------------
QJsonObject Slide::toJson() const
{
    QJsonObject o;
    if (!id.isEmpty()) o.insert(u"id", id);
    putOpt(o, "group", group);
    putOpt(o, "color", color);
    putOpt(o, "globalGroup", globalGroup);
    if (locked) o.insert(u"locked", true);
    QJsonObject st;
    putOpt(st, "template", settings.templateId);
    putOpt(st, "background", settings.background);
    putOpt(st, "color", settings.color);
    putOpt(st, "backgroundImage", settings.backgroundImage);
    if (settings.resolution) st.insert(u"resolution", settings.resolution->toJson());
    o.insert(u"settings", st);
    if (!children.empty()) {
        QJsonArray a;
        for (const auto& c : children) a.append(c);
        o.insert(u"children", a);
    }
    o.insert(u"notes", notes);
    putOpt(o, "key", key);
    QJsonArray itemsArr;
    for (const auto& i : items) itemsArr.append(i.toJson());
    o.insert(u"items", itemsArr);
    if (timeline) o.insert(u"timeline", *timeline);
    if (!customDynamicValues.isEmpty()) o.insert(u"customDynamicValues", customDynamicValues);
    return o;
}
Slide Slide::fromJson(const QJsonObject& o)
{
    Slide s;
    s.id = str(o, "id");
    s.group = optStr(o, "group");
    s.color = optStr(o, "color");
    s.globalGroup = optStr(o, "globalGroup");
    s.locked = boolean(o, "locked");
    if (const auto v = o.constFind(u"settings"); v != o.constEnd() && v->isObject()) {
        const auto st = v->toObject();
        s.settings.templateId = optStr(st, "template");
        s.settings.background = optStr(st, "background");
        s.settings.color = optStr(st, "color");
        s.settings.backgroundImage = optStr(st, "backgroundImage");
        if (const auto r = st.constFind(u"resolution"); r != st.constEnd() && r->isObject())
            s.settings.resolution = Resolution::fromJson(r->toObject());
    }
    s.children = toStringVector(idList(o, "children"));
    s.notes = str(o, "notes");
    s.key = optStr(o, "key");
    if (const auto v = o.constFind(u"items"); v != o.constEnd() && v->isArray()) {
        for (const auto& iv : v->toArray())
            if (iv.isObject()) s.items.push_back(Item::fromJson(iv.toObject()));
    }
    if (const auto v = o.constFind(u"timeline"); v != o.constEnd() && v->isObject())
        s.timeline = v->toObject();
    if (const auto v = o.constFind(u"customDynamicValues"); v != o.constEnd() && v->isObject())
        s.customDynamicValues = v->toObject();
    return s;
}
QString Slide::plainText() const
{
    QStringList parts;
    for (const auto& i : items) {
        const auto t = i.plainText();
        if (!t.isEmpty()) parts << t;
    }
    return parts.join(u'\n');
}

// ---------------------------------------------------------------------------
// Timeline
// ---------------------------------------------------------------------------
QJsonObject TimelineAction::toJson() const
{
    QJsonObject o;
    o.insert(u"id", id);
    o.insert(u"time", time);
    if (duration != 0.0) o.insert(u"duration", duration);
    o.insert(u"name", name);
    putOpt(o, "color", color);
    o.insert(u"type", type);
    if (!data.isEmpty()) o.insert(u"data", data);
    if (!easing.isEmpty()) o.insert(u"easing", easing);
    return o;
}
TimelineAction TimelineAction::fromJson(const QJsonObject& o)
{
    TimelineAction a;
    a.id = str(o, "id");
    a.time = static_cast<Millis>(number(o, "time"));
    a.duration = number(o, "duration");
    a.name = str(o, "name");
    a.color = optStr(o, "color");
    a.type = str(o, "type", "action");
    if (const auto v = o.constFind(u"data"); v != o.constEnd() && v->isObject()) a.data = v->toObject();
    if (const auto v = o.constFind(u"easing"); v != o.constEnd() && v->isObject()) a.easing = v->toObject();
    return a;
}
QJsonObject Timeline::toJson() const
{
    QJsonObject o;
    QJsonArray a;
    for (const auto& act : actions) a.append(act.toJson());
    o.insert(u"actions", a);
    if (loop) o.insert(u"loop", true);
    return o;
}
Timeline Timeline::fromJson(const QJsonObject& o)
{
    Timeline t;
    t.loop = boolean(o, "loop");
    if (const auto v = o.constFind(u"actions"); v != o.constEnd() && v->isArray()) {
        for (const auto& av : v->toArray())
            if (av.isObject()) t.actions.push_back(TimelineAction::fromJson(av.toObject()));
    }
    std::sort(t.actions.begin(), t.actions.end(), [](const auto& a, const auto& b) { return a.time < b.time; });
    return t;
}

// ---------------------------------------------------------------------------
// Show
// ---------------------------------------------------------------------------
QJsonObject Show::toJson() const
{
    QJsonObject o;
    o.insert(u"name", name);
    if (origin) o.insert(u"origin", *origin);
    if (category) o.insert(u"category", *category);
    if (isPrivate) o.insert(u"private", true);
    if (locked) o.insert(u"locked", true);
    if (!quickAccess.isEmpty()) o.insert(u"quickAccess", quickAccess);
    if (!reference.isEmpty()) o.insert(u"reference", reference);

    QJsonObject s;
    s.insert(u"activeLayout", settings.activeLayout);
    if (settings.templateId) s.insert(u"template", *settings.templateId);
    if (settings.customAction) s.insert(u"customAction", *settings.customAction);
    if (!settings.customFonts.isEmpty()) s.insert(u"customFonts", settings.customFonts);
    o.insert(u"settings", s);

    QJsonObject ts;
    ts.insert(u"created", timestamps.created);
    if (timestamps.modified) ts.insert(u"modified", *timestamps.modified);
    if (timestamps.used) ts.insert(u"used", *timestamps.used);
    o.insert(u"timestamps", ts);

    if (!tags.isEmpty()) {
        QJsonArray ta;
        for (const auto& t : tags) ta.append(t);
        QJsonObject md;
        md.insert(u"tags", ta);
        o.insert(u"metadata", md);
    }

    QJsonObject m;
    auto putMeta = [&m](const char* k, const std::optional<QString>& v) { putOpt(m, k, v); };
    putMeta("number", meta.number); putMeta("title", meta.title);
    putMeta("artist", meta.artist); putMeta("author", meta.author);
    putMeta("composer", meta.composer); putMeta("publisher", meta.publisher);
    putMeta("copyright", meta.copyright); putMeta("ccli", meta.ccli);
    putMeta("year", meta.year); putMeta("key", meta.key);
    o.insert(u"meta", m);

    QJsonObject slidesObj;
    for (const auto& [id, s] : slides) slidesObj.insert(id, s.toJson());
    o.insert(u"slides", slidesObj);

    QJsonObject layoutsObj;
    for (const auto& [id, l] : layouts) layoutsObj.insert(id, l.toJson());
    o.insert(u"layouts", layoutsObj);

    QJsonObject mediaObj;
    for (const auto& [id, md] : media) mediaObj.insert(id, md.toJson());
    o.insert(u"media", mediaObj);
    return o;
}

Show Show::fromJson(const QJsonObject& o)
{
    Show s;
    s.name = str(o, "name");
    s.origin = optStr(o, "origin");
    s.category = optStr(o, "category");
    s.isPrivate = boolean(o, "private");
    s.locked = boolean(o, "locked");
    if (const auto v = o.constFind(u"quickAccess"); v != o.constEnd() && v->isObject()) s.quickAccess = v->toObject();
    if (const auto v = o.constFind(u"reference"); v != o.constEnd() && v->isObject()) s.reference = v->toObject();
    if (const auto v = o.constFind(u"settings"); v != o.constEnd() && v->isObject()) {
        const auto st = v->toObject();
        s.settings.activeLayout = str(st, "activeLayout");
        s.settings.templateId = optStr(st, "template");
        s.settings.customAction = optStr(st, "customAction");
        if (const auto f = st.constFind(u"customFonts"); f != st.constEnd() && f->isArray())
            s.settings.customFonts = f->toArray();
    }
    if (const auto v = o.constFind(u"timestamps"); v != o.constEnd() && v->isObject()) {
        const auto ts = v->toObject();
        s.timestamps.created = static_cast<Millis>(number(ts, "created"));
        if (const auto x = optNum(ts, "modified")) s.timestamps.modified = static_cast<Millis>(*x);
        if (const auto x = optNum(ts, "used")) s.timestamps.used = static_cast<Millis>(*x);
    }
    if (const auto v = o.constFind(u"metadata"); v != o.constEnd() && v->isObject())
        s.tags = idList(v->toObject(), "tags");
    if (const auto v = o.constFind(u"meta"); v != o.constEnd() && v->isObject()) {
        const auto m = v->toObject();
        s.meta.number = optStr(m, "number"); s.meta.title = optStr(m, "title");
        s.meta.artist = optStr(m, "artist"); s.meta.author = optStr(m, "author");
        s.meta.composer = optStr(m, "composer"); s.meta.publisher = optStr(m, "publisher");
        s.meta.copyright = optStr(m, "copyright"); s.meta.ccli = optStr(m, "CCLI");
        s.meta.year = optStr(m, "year"); s.meta.key = optStr(m, "key");
    }
    if (const auto v = o.constFind(u"slides"); v != o.constEnd() && v->isObject()) {
        const auto so = v->toObject();
        for (auto i = so.constBegin(); i != so.constEnd(); ++i)
            if (i.value().isObject()) s.slides.insert(i.key(), Slide::fromJson(i.value().toObject()));
    }
    if (const auto v = o.constFind(u"layouts"); v != o.constEnd() && v->isObject()) {
        const auto lo = v->toObject();
        for (auto i = lo.constBegin(); i != lo.constEnd(); ++i)
            if (i.value().isObject()) {
                auto l = Layout::fromJson(i.value().toObject());
                if (l.id.isEmpty()) l.id = i.key();
                s.layouts.insert(i.key(), std::move(l));
            }
    }
    if (const auto v = o.constFind(u"media"); v != o.constEnd() && v->isObject()) {
        const auto mo = v->toObject();
        for (auto i = mo.constBegin(); i != mo.constEnd(); ++i)
            if (i.value().isObject()) {
                auto md = MediaDescriptor::fromJson(i.value().toObject());
                if (md.id.isEmpty()) md.id = i.key();
                s.media.insert(i.key(), std::move(md));
            }
    }
    return s;
}

// ---------------------------------------------------------------------------
// Project / Folder
// ---------------------------------------------------------------------------
QJsonObject ProjectShowRef::toJson() const
{
    QJsonObject o;
    o.insert(u"id", id);
    putOpt(o, "layout", layout);
    putOpt(o, "name", name);
    if (type) o.insert(u"type", showTypeToString(*type));
    putOpt(o, "notes", notes);
    putOpt(o, "color", color);
    if (muted) o.insert(u"muted", true);
    if (loop) o.insert(u"loop", true);
    if (played) o.insert(u"played", true);
    if (!data.isEmpty()) o.insert(u"data", data);
    return o;
}
static ProjectShowRef projectRefFromJson(const QJsonObject& o)
{
    ProjectShowRef r;
    r.id = str(o, "id");
    r.layout = optStr(o, "layout");
    r.name = optStr(o, "name");
    if (const auto t = optStr(o, "type")) r.type = showTypeFromString(*t);
    r.notes = optStr(o, "notes");
    r.color = optStr(o, "color");
    r.muted = boolean(o, "muted");
    r.loop = boolean(o, "loop");
    r.played = boolean(o, "played");
    if (const auto v = o.constFind(u"data"); v != o.constEnd() && v->isObject()) r.data = v->toObject();
    return r;
}

QJsonObject Project::toJson() const
{
    QJsonObject o;
    if (!id.isEmpty()) o.insert(u"id", id);
    o.insert(u"name", name);
    o.insert(u"notes", notes);
    o.insert(u"parent", parent);
    o.insert(u"created", created);
    if (modified) o.insert(u"modified", *modified);
    if (used) o.insert(u"used", *used);
    if (archived) o.insert(u"archived", true);
    QJsonArray a;
    for (const auto& s : shows) a.append(s.toJson());
    o.insert(u"shows", a);
    if (timeline) o.insert(u"timeline", timeline->toJson());
    return o;
}
Project Project::fromJson(const QJsonObject& o)
{
    Project p;
    p.id = str(o, "id");
    p.name = str(o, "name");
    p.notes = str(o, "notes");
    p.parent = str(o, "parent", "root");
    p.created = static_cast<Millis>(number(o, "created"));
    if (const auto x = optNum(o, "modified")) p.modified = static_cast<Millis>(*x);
    if (const auto x = optNum(o, "used")) p.used = static_cast<Millis>(*x);
    p.archived = boolean(o, "archived");
    if (const auto v = o.constFind(u"shows"); v != o.constEnd() && v->isArray()) {
        for (const auto& sv : v->toArray())
            if (sv.isObject()) p.shows.push_back(projectRefFromJson(sv.toObject()));
    }
    if (const auto v = o.constFind(u"timeline"); v != o.constEnd() && v->isObject())
        p.timeline = Timeline::fromJson(v->toObject());
    return p;
}

QJsonObject Folder::toJson() const
{
    QJsonObject o;
    if (!id.isEmpty()) o.insert(u"id", id);
    o.insert(u"name", name);
    o.insert(u"parent", parent);
    o.insert(u"created", created);
    if (modified) o.insert(u"modified", *modified);
    return o;
}
Folder Folder::fromJson(const QJsonObject& o)
{
    Folder f;
    f.id = str(o, "id");
    f.name = str(o, "name");
    f.parent = str(o, "parent", "root");
    f.created = static_cast<Millis>(number(o, "created"));
    if (const auto x = optNum(o, "modified")) f.modified = static_cast<Millis>(*x);
    return f;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
Millis nowMillis()
{
    return QDateTime::currentMSecsSinceEpoch();
}

QString makeId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QStringList splitSections(const QString& text)
{
    QStringList out;
    QString current;
    for (const auto ch : text) {
        if (ch == u'\n') {
            out << current.trimmed();
            current.clear();
        } else {
            current += ch;
        }
    }
    out << current.trimmed();
    out.removeAll({});
    return out;
}

} // namespace ps
