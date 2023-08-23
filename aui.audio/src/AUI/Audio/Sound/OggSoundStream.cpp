//
// Created by dervisdev on 2/9/2023.
//

#include "OggSoundStream.h"
#include "ogg/ogg.h"
#include "AUI/Audio/Stream/FileStream.h"

OggSoundStream::OggSoundStream(_<IFileStream> fis) : mFis(std::move(fis)) {
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

OggSoundStream::~OggSoundStream() {
    ov_clear(&mVorbisFile);
}

AAudioFormat OggSoundStream::info() {
    return {
        .bitRate = static_cast<unsigned int>(mVorbisFile.vi->bitrate_nominal),
        .channelCount =static_cast<uint8_t>(mVorbisFile.vi->channels),
        .sampleRate = static_cast<unsigned int>(mVorbisFile.vi->rate),
        .bitsPerSample = 16
    };
}

size_t OggSoundStream::read(char* dst, size_t size) {
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

void OggSoundStream::rewind() {
    ov_time_seek(&mVorbisFile, 0);
}

_<OggSoundStream> OggSoundStream::load(_<IFileStream> is) {
    return _new<OggSoundStream>(std::move(is));
}
