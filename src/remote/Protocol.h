#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonValue>

namespace ps::remote {

// FreeShow-compatible envelope: { channel, data, id? }
struct Message
{
    QString channel;
    QJsonValue data;
    QString id;
};

QJsonObject toJson(const Message& m);
Message fromJson(const QJsonObject& o);
QByteArray serialize(const Message& m);

namespace Channels {
inline constexpr auto Connection   = "CONNECTION";
inline constexpr auto Disconnect   = "DISCONNECT";
inline constexpr auto Error        = "ERROR";
inline constexpr auto Password     = "PASSWORD";
inline constexpr auto Access       = "ACCESS";
inline constexpr auto Shows        = "SHOWS";
inline constexpr auto Show         = "SHOW";
inline constexpr auto Projects     = "PROJECTS";
inline constexpr auto Project      = "PROJECT";
inline constexpr auto Out          = "OUT";
inline constexpr auto OutData      = "OUT_DATA";
inline constexpr auto Api          = "API";
inline constexpr auto Crdt         = "CRDT";
inline constexpr auto ThemeColors  = "THEME_COLORS";
inline constexpr auto Language     = "LANGUAGE";
} // namespace Channels

// Subset of FreeShow API action ids that ProSlides implements on day one.
namespace Api {
inline constexpr auto GetOutput       = "get_output";
inline constexpr auto GetCleared      = "get_cleared";
inline constexpr auto GetShows        = "get_shows";
inline constexpr auto GetGroups       = "get_groups";
inline constexpr auto GetSlide        = "get_slide";
inline constexpr auto GetThumbnail    = "get_thumbnail";
inline constexpr auto NextSlide       = "next_slide";
inline constexpr auto PreviousSlide   = "previous_slide";
inline constexpr auto IndexSelectSlide = "index_select_slide";
inline constexpr auto IdSelectSlide   = "id_select_slide";
inline constexpr auto RestoreOutput   = "restore_output";
inline constexpr auto ClearAll        = "clear_all";
inline constexpr auto ClearSlide      = "clear_slide";
inline constexpr auto ClearBackground = "clear_background";
inline constexpr auto PlayMedia       = "play_media";
inline constexpr auto TogglePlayingMedia = "toggle_playing_media";
inline constexpr auto StartShow       = "start_show";
inline constexpr auto ChangeLayout    = "change_layout";
} // namespace Api

// {"channel":"API","api":"next_slide","data":{},"send":true}
struct ApiCall
{
    QString api;
    QJsonObject data;
    QString id;
    bool wantsResponse = false;
};

ApiCall apiCallFromJson(const QJsonObject& o);
QJsonObject apiCallToJson(const ApiCall& call);

} // namespace ps::remote
