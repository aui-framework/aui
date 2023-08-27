#pragma once

#include "AUI/Audio/ISoundInputStream.h"

class ISeekableInputStream;
class AUrl;

/**
 * @brief Sound stream for WAV format
 * @ingroup audio
 */
class AWavSoundStream: public ISoundInputStream {
public:
    explicit AWavSoundStream(_<ISeekableInputStream> is);

    AAudioFormat info() override;

    void rewind() override;

    size_t read(char* dst, size_t size) override;

    static _<ISoundInputStream> load(_<ISeekableInputStream> is);
    static _<AWavSoundStream> fromUrl(const AUrl& url);

private:
    struct WavFileHeader {
        char    chunkID[4];
        int32_t chunkSize;
        char    format[4];

        char    subchunk1ID[4];
        int32_t subchunk1Size;
        int16_t audioFormat;
        int16_t numChannels;
        int32_t sampleRate;
        int32_t byteRate;
        int16_t blockAlign;
        int16_t bitsPerSample;

        char    subchunk2ID[4];
        int32_t subchunk2Size;
    };

    static_assert(sizeof(WavFileHeader) == 44);

    _<ISeekableInputStream> mStream;
    WavFileHeader mHeader;
    size_t mChunkReadPos = 0; // до mHeader.ChunkSize
};
