#pragma once

#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Url/AUrl.h"

class ISeekableInputStream;

/**
 * @brief Sound stream for WAV format
 * @ingroup audio
 */
class API_AUI_AUDIO AWavSoundStream: public ISoundInputStream {
public:
    explicit AWavSoundStream(aui::non_null<_<IInputStream>> stream);

    AAudioFormat info() override;

    size_t read(char* dst, size_t size) override;

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

    _<IInputStream> mStream;
    AOptional<AUrl> mUrl;
    WavFileHeader mHeader{};
    size_t mChunkReadPos = 0;

    void readHeader();
};
