#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Url/AUrl.h"

_<IAudioPlayer> IAudioPlayer::fromUrl(const AUrl& url) {
    return IAudioPlayer::fromSoundStream(ISoundInputStream::fromUrl(url));
}
