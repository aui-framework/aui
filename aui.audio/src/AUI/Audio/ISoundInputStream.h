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
