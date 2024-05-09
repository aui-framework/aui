//
// Created by dervisdev on 2/9/2023.
//

#include "AOggSoundStream.h"
#include "ogg/ogg.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"
#include "vorbis/vorbisfile.h"
#include "AUI/Audio/ABadFormatException.h"

AOggSoundStream::OggVorbisFile::OggVorbisFile(_<IInputStream> stream) : mSourceStream(std::move(stream)) {
    AUI_ASSERTX(mSourceStream != nullptr, "provided OggVorbisFile source stream must be non-null");
    ov_callbacks callbacks = {
        .read_func = [](void *ptr, size_t size, size_t nmemb, void *datasource) -> size_t {
            size_t readCount = 0;
            try {
                for (char* current = (char*) ptr; nmemb > 0; current += size, nmemb -= 1) {
                    auto r = reinterpret_cast<IInputStream*>(datasource)->read(current, size);
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

    if (ov_test_callbacks(mSourceStream.get(), &file(), nullptr, 0, callbacks) < 0) {
        throw aui::audio::ABadFormatException("Failed to initialize OggVorbis_File: not an ogg stream");
    }

    if (auto code = ov_test_open(&file()); code < 0) {

        throw AException("Failed to initialize OggVorbis_File: return code {}"_format(code));
    }
}

AOggSoundStream::OggVorbisFile::~OggVorbisFile() {
    ov_clear(&file());
}

size_t AOggSoundStream::OggVorbisFile::read(char* dst, size_t size) {
    size -= size % (aui::audio::bytesPerSample(AOggSoundStream::SAMPLE_FORMAT) * file().vi->channels);
    int currentSection;
    auto end = dst + size;
    auto it = dst;
    while (it < end) {
        size_t len = end - it;
        auto r = ov_read(&file(), it, len, false, 2, true, &currentSection);
        if (r < 0) {
            throw AException("ogg decode error, return code = {}"_format(r));
        }
        if (r == 0) { // end of stream
            break;
        }
        it += r;
    }
    return it - dst;
}

[[nodiscard]]
OggVorbis_File& AOggSoundStream::OggVorbisFile::file() noexcept {
    return mFile.value();
}

[[nodiscard]]
const OggVorbis_File& AOggSoundStream::OggVorbisFile::file() const noexcept {
    return mFile.value();
}

AOggSoundStream::AOggSoundStream(AUrl url) : mUrl(std::move(url)) {
    mVorbisFile.emplace(mUrl->open());
}

AOggSoundStream::AOggSoundStream(_<IInputStream> stream) {
    mVorbisFile.emplace(std::move(stream));
}

AOggSoundStream::~AOggSoundStream() {
    mVorbisFile.reset();
}

AAudioFormat AOggSoundStream::info() {
    if (!mVorbisFile) {
        throw AException("cannot get AOggSoundStream stream info: vorbis file is not opened");
    }
    auto& file = mVorbisFile->file();
    return {
        .channelCount = static_cast<AChannelFormat>(file.vi->channels),
        .sampleRate = static_cast<unsigned int>(file.vi->rate),
        .sampleFormat = SAMPLE_FORMAT
    };
}

size_t AOggSoundStream::read(char* dst, size_t size) {
    if (!mVorbisFile) {
        throw AException("ogg decode error: vorbis file is not opened");
    }

   return mVorbisFile->read(dst, size);
}

void AOggSoundStream::rewind() {
    if (!mUrl) {
        throw AException("cannot rewind AOggSoundStream: url is not provided");
    }
    mVorbisFile.emplace(loadSourceInputStream(*mUrl));
}

_<AOggSoundStream> AOggSoundStream::fromUrl(AUrl url) {
    return _new<AOggSoundStream>(std::move(url));
}
