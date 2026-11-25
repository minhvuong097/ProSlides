#pragma once

// ProSlides core data types.
// Schema is intentionally compatible with FreeShow's document model:
//   Show { slides: {id: Slide}, layouts: {id: Layout}, media: {id: Media}, ... }
//   Show file on disk: JSON tuple [showId, Show]
// Ports of FreeShow fields use std::optional to preserve the "field present" bit.

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QStringList>
#include <QColor>
#include <QRectF>
#include <QSize>
#include <QMetaType>

#include <cstdint>
#include <optional>
#include <vector>
#include <map>
#include <unordered_map>

namespace ps {

using Id    = QString;
using Millis = qint64;

// ---------------------------------------------------------------------------
// Enumerations (string mapping lives in Types.cpp)
// ---------------------------------------------------------------------------
enum class ItemType {
    Text, Media, Image, Video, Audio, Camera, Timer, Clock, Button, Events,
    Weather, Variable, Web, QrCode, Icon, SlideTracker, Visualizer, Captions,
    Metronome, CurrentOutput, Chart, Table, Effect, Screen, Ndi, Player,
    Overlay, Pdf, Ppt, Folder, Divider, Show, Section, Placeholder, Unknown
};
QString itemTypeToString(ItemType t);
ItemType itemTypeFromString(const QString& s);

enum class ShowType {
    Divider, Show, Image, Video, Audio, Player, Section, Overlay, Effect,
    Pdf, Ppt, Screen, Ndi, Camera, Folder, ShowPlaceholder
};
QString showTypeToString(ShowType t);
ShowType showTypeFromString(const QString& s);

enum class TransitionType { None, Blur, Fade, Crossfade, Fly, Scale, Slide, Spin };
QString transitionTypeToString(TransitionType t);
TransitionType transitionTypeFromString(const QString& s);

enum class MediaType { Media, Video, Image, Effect, Screen, Ndi, Omt, Camera, Player, Audio };
QString mediaTypeToString(MediaType t);
MediaType mediaTypeFromString(const QString& s);

// ---------------------------------------------------------------------------
// Small value types
// ---------------------------------------------------------------------------
struct Resolution {
    int width  = 1920;
    int height = 1080;

    QJsonObject toJson() const;
    static Resolution fromJson(const QJsonObject& o);
};

// FreeShow: Cropping { top, right, bottom, left, type?: "clip"|"ppt" }
struct Cropping {
    double top = 0, right = 0, bottom = 0, left = 0;
    QString type; // "clip" | "ppt" | empty
};

// FreeShow: Chords { id, pos, key }
struct Chord {
    Id     id;
    double pos = 0; // character offset within the text line
    QString key;    // e.g. "C", "G/B"

    QJsonObject toJson() const;
};

// FreeShow: Line.text[] — styled text fragments
struct TextPart {
    QString value;
    QString style;                     // inline CSS (color, font-weight, ...)
    std::optional<QString> customType;
    std::optional<QString> sourceDynamicKey;

    QJsonObject toJson() const;
};

// FreeShow: Line
struct Line {
    QString align = "center";
    std::vector<TextPart> text;
    std::vector<Chord> chords;

    QJsonObject toJson() const;
    QString plainText() const; // concatenate all fragments
};

// FreeShow: List / ListItem
struct ListItem {
    QString text;
    std::optional<QString> icon;

    QJsonObject toJson() const;
};
struct ListData {
    bool enabled = false;
    std::optional<QString> style;
    std::optional<int> interval;
    std::vector<ListItem> items;

    QJsonObject toJson() const;
};

// FreeShow: Scrolling
struct Scrolling {
    QString type = "none"; // none|top_bottom|bottom_top|left_right|right_left
    std::optional<double> speed;
    std::optional<double> gap;

    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Item — a single content element of a slide (FreeShow Item, trimmed to the
// fields ProSlides uses; unknown JSON is preserved in `extra`).
// ---------------------------------------------------------------------------
struct Item {
    Id id;
    ItemType type = ItemType::Text;

    QString style;                       // inline CSS for the whole item
    std::optional<QString> align;
    std::optional<QString> specialStyle; // stored as JSON string in FreeShow
    std::vector<Line> lines;
    std::optional<ListData> list;

    std::optional<QString> textFit;      // autosize mode
    std::optional<int> autoFontSize;

    // Visual editor geometry (FreeShow stores this inside the CSS style;
    // ProSlides keeps it as explicit fields and also mirrors it into style).
    double x = 0.0;
    double y = 0.0;
    double width = 900.0;
    double height = 160.0;
    int fontSize = 52;
    QString color = QStringLiteral(u"#f5f5f5");
    QString alignSelf = QStringLiteral(u"center");

    // media items
    std::optional<QString> src;
    std::optional<QString> customSvg;
    std::optional<QString> filter;
    std::optional<QString> blend;
    std::optional<Cropping> cropping;
    bool muted = false;
    bool loop = false;
    double volume = 1.0;
    double speed = 1.0;
    std::optional<double> fromTime;
    std::optional<double> toTime;
    std::optional<QString> fit = "contain";

    // interaction / behaviour
    bool decoration = false;
    bool clickReveal = false;
    bool lineReveal = false;
    std::vector<QString> bindings;
    QJsonObject actions;
    std::optional<Scrolling> scrolling;

    // chords overlay settings stored as JSON
    QJsonObject chordsStyle;

    // language / scripture metadata
    std::optional<QString> language;
    std::optional<QString> timerId;
    QJsonObject timer;
    QJsonObject clock;
    QJsonObject events;
    QJsonObject qrCode;
    QJsonObject weather;
    QJsonObject table;
    QJsonObject chart;

    // FreeShow keeps many extension fields; preserve them verbatim.
    QJsonObject extra;

    QJsonObject toJson() const;
    QString plainText() const;
    static Item fromJson(const QJsonObject& o);
};

// ---------------------------------------------------------------------------
// Media descriptor (Show.media entry)
// ---------------------------------------------------------------------------
struct MediaDescriptor {
    Id id;
    QString name;
    QString path;
    MediaType type = MediaType::Media;
    std::optional<QString> videoType;  // "foreground" | "background"
    std::optional<QString> cameraGroup;
    bool muted = false;
    bool loop = false;
    std::optional<QString> filters;
    std::optional<QString> base64;
    QJsonObject cloud;

    QJsonObject toJson() const;
    static MediaDescriptor fromJson(const QJsonObject& o);
};

// ---------------------------------------------------------------------------
// Transition
// ---------------------------------------------------------------------------
struct TransitionData {
    TransitionType type = TransitionType::None;
    double duration = 0.0; // seconds
    QString easing = "linear";
    std::optional<double> delay;
};
struct Transition {
    TransitionType type = TransitionType::None;
    double duration = 0.0;
    QString easing = "linear";
    std::optional<TransitionData> in;
    std::optional<TransitionData> out;
    std::optional<TransitionData> between;
    std::optional<QString> custom;

    QJsonObject toJson() const;
    static Transition fromJson(const QJsonValue& v);
};

// ---------------------------------------------------------------------------
// SlideAction / SlideData / Layout
// ---------------------------------------------------------------------------
struct SlideAction {
    Id id;
    QString name;
    std::vector<QString> triggers;
    QJsonObject actionValues;
    QJsonObject customData;

    QJsonObject toJson() const;
};

struct SlideData {
    Id id;                              // referenced Slide id
    bool disabled = false;
    std::optional<Id> parent;
    std::vector<Id> children;
    std::optional<QString> color;
    std::optional<int> nextTimer;
    std::optional<Transition> transition;
    std::optional<QString> filter;
    bool end = false;
    std::optional<int> timer;
    std::optional<QString> background;
    std::vector<Id> overlays;
    std::vector<Id> effects;
    std::vector<Id> audio;
    bool remove = false;
    std::vector<SlideAction> actions;
    std::vector<QString> bindings;
    std::optional<int> breakDuration;

    QJsonObject toJson() const;
    static SlideData fromJson(const QJsonObject& o);
};

struct Layout {
    Id id;
    QString name;
    QString notes;
    std::vector<SlideData> slides;
    std::optional<QJsonObject> timeline; // parsed on demand by timeline module

    QJsonObject toJson() const;
    static Layout fromJson(const QJsonObject& o);
};

// ---------------------------------------------------------------------------
// Slide
// ---------------------------------------------------------------------------
struct Slide {
    Id id;
    std::optional<QString> group;
    std::optional<QString> color;
    std::optional<QString> globalGroup;
    bool locked = false;

    struct Settings {
        std::optional<Id> templateId;
        std::optional<QString> background;
        std::optional<QString> color;
        std::optional<QString> backgroundImage;
        std::optional<Resolution> resolution;
    } settings;

    std::vector<Id> children;
    QString notes;
    std::vector<Item> items;
    std::optional<QString> key; // musical key (shown in the chord strip)

    // per-slide timeline (actions), normalized: time = ms, duration = seconds
    std::optional<QJsonObject> timeline;

    QJsonObject customDynamicValues;

    QJsonObject toJson() const;
    static Slide fromJson(const QJsonObject& o);

    QString plainText() const;
};

// ---------------------------------------------------------------------------
// Timeline (Show/Layout/Project level) — FreeShow Timeline/TimelineAction
// ---------------------------------------------------------------------------
struct TimelineAction {
    Id id;
    Millis time = 0;          // milliseconds from start
    double duration = 0.0;    // seconds (FreeShow quirk: UI stores seconds)
    QString name;
    std::optional<QString> color;
    QString type;             // action|slide|show|audio|style
    QJsonObject data;
    QJsonObject easing;

    QJsonObject toJson() const;
    static TimelineAction fromJson(const QJsonObject& o);
};
struct Timeline {
    std::vector<TimelineAction> actions;
    bool loop = false;

    QJsonObject toJson() const;
    static Timeline fromJson(const QJsonObject& o);
};

// ---------------------------------------------------------------------------
// Show
// ---------------------------------------------------------------------------
struct ShowMeta {
    std::optional<QString> number, title, artist, author, composer, publisher;
    std::optional<QString> copyright, ccli, year, key;
};

struct ShowSettings {
    Id activeLayout;
    std::optional<Id> templateId;
    std::optional<QString> customAction;
    QJsonArray customFonts;
};

struct ShowTimestamps {
    Millis created = 0;
    std::optional<Millis> modified;
    std::optional<Millis> used;
};

struct Show {
    Id id;
    QString name;
    std::optional<QString> origin;
    std::optional<QString> category;
    bool isPrivate = false;
    bool locked = false;
    QJsonObject quickAccess;
    QJsonObject reference;
    ShowSettings settings;
    ShowTimestamps timestamps;
    QStringList tags;
    ShowMeta meta;

    std::map<Id, Slide> slides;
    std::map<Id, Layout> layouts;
    std::map<Id, MediaDescriptor> media;

    QJsonObject toJson() const;              // Show object only (no id)
    static Show fromJson(const QJsonObject& o);
};

// ---------------------------------------------------------------------------
// Project / service flow — FreeShow Project + ProjectShowRef
// ---------------------------------------------------------------------------
struct ProjectShowRef {
    Id id;
    std::optional<Id> layout;
    std::optional<QString> name;
    std::optional<ShowType> type;
    std::optional<QString> notes;
    std::optional<QString> color;
    bool muted = false;
    bool loop = false;
    bool played = false;
    QJsonObject data;

    QJsonObject toJson() const;
};

struct Project {
    Id id;
    QString name;
    QString notes;
    Id parent = "root";
    Millis created = 0;
    std::optional<Millis> modified;
    std::optional<Millis> used;
    bool archived = false;
    std::vector<ProjectShowRef> shows;
    std::optional<Timeline> timeline;

    QJsonObject toJson() const;
    static Project fromJson(const QJsonObject& o);
};

struct Folder {
    Id id;
    QString name;
    Id parent = "root";
    Millis created = 0;
    std::optional<Millis> modified;
    QJsonObject toJson() const;
    static Folder fromJson(const QJsonObject& o);
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
Millis nowMillis();
QString makeId();                       // UUID v4 string
QStringList splitSections(const QString& text);

} // namespace ps

Q_DECLARE_METATYPE(ps::TimelineAction)
