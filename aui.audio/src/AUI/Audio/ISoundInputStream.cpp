#include "ISoundInputStream.h"
#include "AUI/Audio/Formats/wav/AWavSoundStream.h"
#include "AUI/Audio/Formats/ogg/AOggSoundStream.h"
#include "AUI/IO/AByteBufferInputStream.h"
#include <vorbis/vorbisfile.h>
#include "AUI/Audio/ABadFormatException.h"
#include "AUI/Logging/ALogger.h"

_<ISoundInputStream> ISoundInputStream::fromUrl(const AUrl& url) {

    try {
        return _new<AWavSoundStream>(Cache::get(url));
    }
    catch (const aui::audio::ABadFormatException&) {
    }
    catch(...) {
        throw;
    }

    try {
        return _new<AOggSoundStream>(Cache::get(url));
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

_unique<IInputStream> ISoundInputStream::Cache::get(const AUrl& key) {
    if (auto c = inst().mEntries.contains(key)) {
        return std::make_unique<AStrongByteBufferInputStream>(c->second);
    }
    try {
        auto stream = key.open();
        if (auto file = dynamic_cast<AFileInputStream*>(stream.get()); file && file->size() < MAX_FILE_SIZE_TO_CACHE) {
            return std::make_unique<AStrongByteBufferInputStream>(inst().mEntries[key] = _new<AByteBuffer>(AByteBuffer::fromStream(file)));
        }
        return stream;
    }
    catch(...) {
    }

    return nullptr;
}
