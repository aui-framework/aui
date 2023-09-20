#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Url/AUrl.h"

#if AUI_PLATFORM_LINUX
#include "linux/APulseAudioPlayer.h"
using PlatformAudioPlayer = APulseAudioPlayer;
#elif AUI_PLATFORM_ANDROID
#include "android/AOboeAudioPlayer.h"
using PlatformAudioPlayer = AOboeAudioPlayer;
#endif

//_<IAudioPlayer> IAudioPlayer::fromUrl(const AUrl& url) {
//    auto result = _new<PlatformPlayerType>();
//    result->setSource(url.open());
//}

_<IAudioPlayer> IAudioPlayer::fromSoundStream(_<ISoundInputStream> stream) {
    auto result = _new<PlatformAudioPlayer>();
    result->setSource(std::move(stream));
    return result;
}
