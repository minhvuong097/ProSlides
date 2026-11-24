#include "VideoEngine.h"

namespace ps::media {

QString playbackStateName(PlaybackState state)
{
    switch (state) {
    case PlaybackState::Stopped: return QStringLiteral(u"stopped");
    case PlaybackState::Playing: return QStringLiteral(u"playing");
    case PlaybackState::Paused:  return QStringLiteral(u"paused");
    case PlaybackState::Ended:   return QStringLiteral(u"ended");
    }
    return {};
}

} // namespace ps::media
