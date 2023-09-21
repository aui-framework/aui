#include "ISoundInputStream.h"
#include "AUI/Audio/Formats/wav/AWavSoundStream.h"
#include "AUI/Audio/Formats/ogg/AOggSoundStream.h"
#include <vorbis/vorbisfile.h>

_<ISoundInputStream> ISoundInputStream::fromUrl(const AUrl& url) {
    try {
        return AWavSoundStream::fromUrl(url);
    }
    catch(...) { }

    try {
        return AOggSoundStream::fromUrl(url);
    }
    catch(...) { }

    return nullptr;
}
