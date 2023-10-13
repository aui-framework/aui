#include "ISoundInputStream.h"
#include "AUI/Audio/Formats/wav/AWavSoundStream.h"
#include "AUI/Audio/Formats/ogg/AOggSoundStream.h"
#include "AUI/IO/AByteBufferInputStream.h"
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

ISoundInputStream::Cache& ISoundInputStream::Cache::inst() {
    static ISoundInputStream::Cache inst;
    return inst;
}

_<IInputStream> ISoundInputStream::getInputStream(const AUrl &key) {
    if (auto result = Cache::get(key)) {
        return _new<AByteBufferInputStream>(*result);
    }

    try {
        return key.open();
    }
    catch (...) {
    }

    return nullptr;
}

_<AByteBuffer> ISoundInputStream::Cache::load(const AUrl &key) {
    try {
        auto stream = key.open();
        if (auto file = _cast<AFileInputStream>(stream); file && file->size() < MAX_FILE_SIZE_TO_CACHE) {
            return _new<AByteBuffer>(AByteBuffer::fromStream(*file));
        }
    }
    catch(...) {
    }

    return nullptr;
}
