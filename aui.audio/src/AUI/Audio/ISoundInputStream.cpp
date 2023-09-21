#include "ISoundInputStream.h"
#include "Formats/AWavSoundStream.h"
#include "Formats/AOggSoundStream.h"

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
