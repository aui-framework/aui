#pragma once

#include "AUI/Common/AByteBuffer.h"
#include "Sound.h"
#include "AUI/Audio/Stream/FileStream.h"

typedef struct wavfile_header_s
{
    char    ChunkID[4];     /*  4   */
    int32_t ChunkSize;      /*  4   */
    char    Format[4];      /*  4   */

    char    Subchunk1ID[4]; /*  4   */
    int32_t Subchunk1Size;  /*  4   */
    int16_t AudioFormat;    /*  2   */
    int16_t NumChannels;    /*  2   */
    int32_t SampleRate;     /*  4   */
    int32_t ByteRate;       /*  4   */
    int16_t BlockAlign;     /*  2   */
    int16_t BitsPerSample;  /*  2   */

    char    Subchunk2ID[4];
    int32_t Subchunk2Size;
} wavfile_header_t;

class WavSoundStream: public Audio::SoundStream {
private:
    std::shared_ptr<IFileStream> mFis;
    wavfile_header_t mHeader;
    size_t mChunkReadPos = 0; // до mHeader.ChunkSize
public:
    WavSoundStream(std::shared_ptr<IFileStream> is): mFis(std::move(is)) {
        mFis->read(reinterpret_cast<char*>(&mHeader), sizeof(mHeader));

        if (std::memcmp(mHeader.ChunkID, "RIFF", 4) != 0 ||
            std::memcmp(mHeader.Format, "WAVE", 4) != 0 ||
            std::memcmp(mHeader.Subchunk1ID, "fmt ", 4) != 0) {
            throw std::runtime_error("not a wav file");
        }
    }

    AAudioFormat info() override {
        return AAudioFormat {
                static_cast<unsigned int>(mHeader.ByteRate * 2),
                static_cast<uint8_t>(mHeader.NumChannels),
                static_cast<unsigned int>(mHeader.SampleRate),
                static_cast<uint8_t>(mHeader.BitsPerSample)
        };
    }

    bool isEof() override {
        return false;
    }

    void rewind() override {
        mChunkReadPos = 0;
        mFis->seek(sizeof(mHeader), std::ios::beg);
    }

    size_t read(char* dst, size_t size) override {
        auto remaining = mHeader.ChunkSize - mChunkReadPos;
        if (remaining == 0) return 0;
        size_t r = mFis->read(dst, std::min(size, remaining));
        mChunkReadPos += r;
        return r;
    }

    static std::shared_ptr<Audio::SoundStream> load(std::shared_ptr<IFileStream> is) {
        return std::make_shared<WavSoundStream>(std::move(is));
    }
};
