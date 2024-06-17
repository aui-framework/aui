#pragma once

#include "AUI/IO/IInputStream.h"
#include "AUI/Audio/AAudioFormat.h"
#include "AUI/Util/Cache.h"

class AUrl;

/**
 * @brief Base interface for representing sound input streams of different formats
 * @ingroup audio
 */
class ISoundInputStream: public IInputStream {
public:
    static _<ISoundInputStream> fromUrl(const AUrl& url);

    /**
     * @brief Get general info about sound stream
     * @return AAudioFormat containing info about sound stream
     */
    virtual AAudioFormat info() = 0;

protected:
    static _<IInputStream> loadSourceInputStream(const AUrl& key);

private:
    static constexpr size_t MAX_FILE_SIZE_TO_CACHE = 1 << 24;
    class Cache : public ::Cache<AByteBuffer, Cache, AUrl> {
    public:
        static Cache& inst();
    protected:
        _<AByteBuffer> load(const AUrl& key) override;
    };
};
