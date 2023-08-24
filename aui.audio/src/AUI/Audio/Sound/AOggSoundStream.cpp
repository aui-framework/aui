//
// Created by dervisdev on 2/9/2023.
//

#include "AOggSoundStream.h"
#include "ogg/ogg.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"

AOggSoundStream::AOggSoundStream(_<ISeekableInputStream> fis) : mStream(std::move(fis)) {
    OggVorbis_File vorbisFile;
    ov_callbacks callbacks = {
            [](void *ptr, size_t size, size_t nmemb, void *datasource) -> size_t { // read
                size_t readCount = 0;
                try {
                    for (char* current = (char*) ptr; nmemb > 0; current += size, nmemb -= 1) {
                        auto r = reinterpret_cast<AOggSoundStream*>(datasource)->mStream->read(current, size);
                        if (r != size) break;
                        readCount += 1;
                    }
                } catch (...) {

                }
                return readCount * size;
            },
            [](void *datasource, ogg_int64_t offset, int whence) -> int { // seek
                auto b = reinterpret_cast<AOggSoundStream*>(datasource);
                switch (whence) {
                    case SEEK_SET: b->mStream->seek(offset, std::ios::beg); break;
                    case SEEK_END: b->mStream->seek(offset, std::ios::end); break;
                    case SEEK_CUR: b->mStream->seek(offset, std::ios::cur); break;
                    default: assert(0);
                }
                return 0;
            },
            [](void*) { return 0; }, // close
            [](void* datasource) -> long { //tell
                auto b = reinterpret_cast<AOggSoundStream*>(datasource);
                return b->mStream->tell();
            }
    };

    if (ov_open_callbacks(this, &vorbisFile, nullptr, 0, callbacks) < 0) {
        throw std::runtime_error("not a ogg stream");
    }

    mVorbisFile = vorbisFile;

}

AOggSoundStream::~AOggSoundStream() {
    ov_clear(&mVorbisFile);
}

AAudioFormat AOggSoundStream::info() {
    return {
        .bitRate = static_cast<unsigned int>(mVorbisFile.vi->bitrate_nominal),
        .channelCount =static_cast<uint8_t>(mVorbisFile.vi->channels),
        .sampleRate = static_cast<unsigned int>(mVorbisFile.vi->rate),
        .bitsPerSample = 16
    };
}

size_t AOggSoundStream::read(char* dst, size_t size) {
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

void AOggSoundStream::rewind() {
    ov_time_seek(&mVorbisFile, 0);
}

_<AOggSoundStream> AOggSoundStream::load(_<ISeekableInputStream> is) {
    return _new<AOggSoundStream>(std::move(is));
}

_<AOggSoundStream> AOggSoundStream::fromUrl(const AUrl& url) {
    return _new<AOggSoundStream>(AStrongByteBufferInputStream::fromUrl(url));
}
