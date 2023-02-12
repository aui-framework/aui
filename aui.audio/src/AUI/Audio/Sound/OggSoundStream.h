//
// Created by dervisdev on 2/9/2023.
//

#pragma once

#include <utility>

#include "Sound.h"
#include "vorbis/vorbisfile.h"
#include "AUI/Audio/Stream/FileStream.h"
#include "ogg/ogg.h"
#include "ogg/os_types.h"

class OggSoundStream: public Audio::SoundStream {
public:
    OggSoundStream(std::shared_ptr<IFileStream> fis) : mFis(std::move(fis)) {
        OggVorbis_File vorbisFile;
        ov_callbacks callbacks = {
                [](void *ptr, size_t size, size_t nmemb, void *datasource) -> size_t { // read
                    size_t readCount = 0;
                    try {
                        for (char* current = (char*) ptr; nmemb > 0; current += size, nmemb -= 1) {
                            auto r = reinterpret_cast<OggSoundStream*>(datasource)->mFis->read(current, size);
                            if (r != size) break;
                            readCount += 1;
                        }
                    } catch (...) {

                    }
                    return readCount * size;
                },
                [](void *datasource, ogg_int64_t offset, int whence) -> int { // seek
                    auto b = reinterpret_cast<OggSoundStream*>(datasource);
                    switch (whence) {
                        case SEEK_SET: b->mFis->seek(offset, std::ios::beg); break;
                        case SEEK_END: b->mFis->seek(offset, std::ios::end); break;
                        case SEEK_CUR: b->mFis->seek(offset, std::ios::cur); break;
                        default: assert(0);
                    }
                    return 0;
                },
                [](void*) { return 0; }, // close
                [](void* datasource) -> long { //tell
                    auto b = reinterpret_cast<OggSoundStream*>(datasource);
                    return b->mFis->tell();
                }
        };

        if (ov_open_callbacks(this, &vorbisFile, nullptr, 0, callbacks) < 0) {
            throw std::runtime_error("not a ogg stream");
        }

        mVorbisFile = vorbisFile;

    }

    ~OggSoundStream() override {
        ov_clear(&mVorbisFile);
    }

    bool isEof() override {
        return false;
    }

    AAudioFormat info() override {
        return { static_cast<unsigned int>(mVorbisFile.vi->bitrate_nominal),
                 static_cast<uint8_t>(mVorbisFile.vi->channels),
                 static_cast<unsigned int>(mVorbisFile.vi->rate),
                 16
        };
    }

    size_t read(char* dst, size_t size) override {
        int currentSection;
        char* end = dst + size;
        for (auto begin = dst; begin != end; ) {
            size_t len = end - begin;
            auto r = ov_read(&mVorbisFile, begin, len, false, 2, true, &currentSection);
            if (r < 0) {
                throw std::runtime_error("ogg decode error");
            }
            if (r == 0) {
                return dst - begin;
            }
            begin += r;
        }
        return size;
    }

    void rewind() override {
        ov_time_seek(&mVorbisFile, 0);
    }

    static std::shared_ptr<Audio::SoundStream> load(std::shared_ptr<IFileStream> is) {
        return std::make_shared<OggSoundStream>(std::move(is));
    }

private:
    std::shared_ptr<IFileStream> mFis;
    OggVorbis_File mVorbisFile;
};

