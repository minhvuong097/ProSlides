#include "VideoEngine.h"

namespace ps::media {

QString playbackStateName(PlaybackState state)
{
    switch (state) {
    case PlaybackState::Stopped: return u"stopped";
    case PlaybackState::Playing: return u"playing";
    case PlaybackState::Paused:  return u"paused";
    case PlaybackState::Ended:   return u"ended";
    }
    return {};
}

} // namespace ps::media
