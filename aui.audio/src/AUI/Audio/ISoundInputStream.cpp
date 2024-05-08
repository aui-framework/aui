#include "ISoundInputStream.h"
#include "AUI/Audio/Formats/wav/AWavSoundStream.h"
#include "../../../../../../../.aui/repo/aui/as/87488ac/aui/aui.audio/src/AUI/Audio/Formats/ogg/AOggSoundStream.h"
#include "AUI/IO/AByteBufferInputStream.h"
#include <vorbis/vorbisfile.h>
#include "AUI/Audio/ABadFormatException.h"
#include "AUI/Logging/ALogger.h"

_<ISoundInputStream> ISoundInputStream::fromUrl(const AUrl& url) {
    try {
        return AWavSoundStream::fromUrl(url);
    }
    catch (const aui::audio::ABadFormatException&) {
    }
    catch(...) {
        throw;
    }

    try {
        return AOggSoundStream::fromUrl(url);
    }
    catch (const aui::audio::ABadFormatException&) {
    }
    catch(...) {
        throw;
    }

    throw AException("Failed to create ISoundInputStream from url = {}: unsupported format"_format(url.full()));
}

ISoundInputStream::Cache& ISoundInputStream::Cache::inst() {
    static ISoundInputStream::Cache inst;
    return inst;
}

_<IInputStream> ISoundInputStream::loadSourceInputStream(const AUrl &key) {
    if (auto result = Cache::get(key)) {
        return _new<AByteBufferInputStream>(*result);
    }

    return key.open();
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
