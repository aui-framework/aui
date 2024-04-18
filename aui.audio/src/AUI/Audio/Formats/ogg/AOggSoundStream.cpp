//
// Created by dervisdev on 2/9/2023.
//

#include "AOggSoundStream.h"
#include "ogg/ogg.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"
#include "vorbis/vorbisfile.h"
#include "AUI/Audio/ABadFormatException.h"

AOggSoundStream::AOggSoundStream(AUrl url) : mUrl(std::move(url)) {
    mStream = mUrl->open();
    initialize();
}

AOggSoundStream::AOggSoundStream(_<IInputStream> stream) : mStream(std::move(stream)) {
    initialize();
}

AOggSoundStream::~AOggSoundStream() {
    clear();
}

AAudioFormat AOggSoundStream::info() {
    if (!mVorbisFile) {
        throw AException("cannot get AOggSoundStream stream info: vorbis file is not opened");
    }
    return {
        .channelCount = static_cast<AChannelFormat>((*mVorbisFile)->vi->channels),
        .sampleRate = static_cast<unsigned int>((*mVorbisFile)->vi->rate),
        .sampleFormat = SAMPLE_FORMAT
    };
}

size_t AOggSoundStream::read(char* dst, size_t size) {
    if (!mVorbisFile) {
        throw AException("ogg decode error: vorbis file is not opened");
    }

    size -= size % (aui::audio::bytesPerSample(SAMPLE_FORMAT) * (*mVorbisFile)->vi->channels);
    int currentSection;
    char* end = dst + size;
    for (auto begin = dst; begin != end; ) {
        size_t len = end - begin;
        auto r = ov_read(mVorbisFile->ptr(), begin, len, false, 2, true, &currentSection);
        if (r < 0) {
            throw AException("ogg decode error");
        }
        if (r == 0) {
            return begin - dst;
        }
        begin += r;
    }
    return size;
}

void AOggSoundStream::rewind() {
    if (!mUrl) {
        throw AException("cannot rewind AOggSoundStream: url is not provided");
    }
    mStream = getInputStream(*mUrl);
    if (mStream) {
        initialize();
    }
}

_<AOggSoundStream> AOggSoundStream::fromUrl(AUrl url) {
    return _new<AOggSoundStream>(std::move(url));
}

void AOggSoundStream::initialize() {
    OggVorbis_File vorbisFile;
    ov_callbacks callbacks = {
            .read_func = [](void *ptr, size_t size, size_t nmemb, void *datasource) -> size_t {
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
            .seek_func = nullptr,
            .close_func = nullptr,
            .tell_func = nullptr
    };

    if (ov_open_callbacks(this, &vorbisFile, nullptr, 0, callbacks) < 0) {
        throw aui::audio::ABadFormatException("not an ogg stream");
    }

    clear();
    mVorbisFile = vorbisFile;
}

void AOggSoundStream::clear() {
    if (mVorbisFile) {
        ov_clear(mVorbisFile->ptr());
        mVorbisFile.reset();
    }
}
