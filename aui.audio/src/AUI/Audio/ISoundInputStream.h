#pragma once

#include "AUI/IO/IInputStream.h"
#include "AUI/Audio/AAudioFormat.h"
#include "AUI/Util/Cache.h"

class AUrl;

/**
 * @brief Base interface for representing sound input streams of different formats
 * @ingroup audio
 * @details
 * <!-- aui:experimental -->
 *
 * An ISoundInputStream is an [IInputStream] of raw PCM samples plus a description of them (`info`). Implement it to
 * feed [IAudioPlayer] with generated or custom decoded audio, see [audio-custom-sources].
 *
 * `ISoundInputStream::fromUrl` opens a url and picks a decoder (WAV, then Ogg Vorbis). It throws
 * `aui::audio::ABadFormatException` if neither fits. Small local files (up to 100 KiB) are cached in memory.
 */
class ISoundInputStream: public IInputStream {
public:
    static _<ISoundInputStream> fromUrl(const AUrl& url);

    /**
     * @brief Get general info about sound stream
     * @return AAudioFormat containing info about sound stream
     */
    virtual AAudioFormat info() = 0;

private:
    static constexpr size_t MAX_FILE_SIZE_TO_CACHE = 100 /* kb */ * 1024;
    class Cache {
    public:
        static Cache& inst();

        static _unique<IInputStream> get(const AUrl& key);

    private:
        AMap<AUrl, _<AByteBuffer>> mEntries;
    };
};
