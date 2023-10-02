#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Url/AUrl.h"

#if AUI_PLATFORM_WIN
#include "win32/DirectSoundAudioPlayer.h"
using DefaultSystemPlayer = DirectSoundAudioPlayer;
#elif AUI_PLATFORM_LINUX
#include "linux/PulseAudioPlayer.h"
using DefaultSystemPlayer = PulseAudioPlayer;
#elif AUI_PLATFORM_ANDROID
#include "android/OboeAudioPlayer.h"
using DefaultSystemPlayer = OboeAudioPlayer;
#elif AUI_PLATFORM_APPLE
#include "apple/CoreAudioPlayer.h"
using DefaultSystemPlayer = CoreAudioPlayer;
#endif

_<IAudioPlayer> IAudioPlayer::fromUrl(const AUrl& url) {
    return IAudioPlayer::fromSoundStream(ISoundInputStream::fromUrl(url));
}

_<IAudioPlayer> IAudioPlayer::fromSoundStream(_<ISoundInputStream> stream) {
    if (!stream) {
        return nullptr;
    }

    auto result = _new<DefaultSystemPlayer>();
    result->setSource(std::move(stream));
    return result;
}
