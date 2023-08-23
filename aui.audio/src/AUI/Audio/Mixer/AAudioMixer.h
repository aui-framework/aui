#pragma once

#include "AUI/Audio/Sound/ISoundStream.h"
#include <mutex>
#include <list>

enum class SampleFormat {
    I16,
    I24,
};

namespace {
    namespace detail {
        template<int power, typename T>
        constexpr T mul_by_2_power(T value) {
            if constexpr(power > 0) {
                return value << power;
            } else {
                return value >> -power;
            }
        }
    }

#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_APPLE
    constexpr SampleFormat OUTPUT_FORMAT = SampleFormat::I16;
#else
    constexpr SampleFormat OUTPUT_FORMAT = SampleFormat::I24;
#endif

    template<SampleFormat f>
    struct sample_type;

    template<>
    struct sample_type<SampleFormat::I16> {
        using type = uint16_t;
        constexpr static int size_bits = 16;
    };

    template<>
    struct sample_type<SampleFormat::I24> {
        using type = uint32_t;
        constexpr static int size_bits = 24;
    };

    template<SampleFormat f>
    constexpr int size_bytes() {
        return sample_type<f>::size_bits / 8;
    }

    template<SampleFormat f>
    using sample_type_t = typename sample_type<f>::type;


    template<SampleFormat f>
    constexpr sample_type_t<f> max_value() {
        return detail::mul_by_2_power<sample_type<f>::size_bits>(sample_type_t<f>(1)) - 1;
    }

#pragma pack(push, 1)
    template<SampleFormat f>
    struct packed_accessor {
        sample_type_t<f> value : sample_type<f>::size_bits;
        unsigned _pad : (32 - sample_type<f>::size_bits);
    };
#pragma pack(pop)
}

class SampleConsumer {
private:
    char* mDestinationBufferBegin;
    char* mDestinationBufferEnd;
    char* mDestinationBufferIt;

public:
    SampleConsumer(char* destinationBufferBegin,
                   char* destinationBufferEnd):
            mDestinationBufferBegin(destinationBufferBegin),
            mDestinationBufferEnd(destinationBufferEnd),
            mDestinationBufferIt(destinationBufferBegin) {}

    template<SampleFormat f>
    inline void commitSample(sample_type_t<f> sample) {
        assert(("buffer overrun", mDestinationBufferIt <= mDestinationBufferEnd));
        using output_t = sample_type<OUTPUT_FORMAT>;
        output_t::type sampleResampled = detail::mul_by_2_power<output_t::size_bits - sample_type<f>::size_bits>(sample);

        auto& pAccessor = *reinterpret_cast<packed_accessor<OUTPUT_FORMAT>*>(mDestinationBufferIt);

        // избегаю здесь memcpy чтобы записывать 3 байта одной инструкцией, вместо 10 которые были бы при memcpy
        // сбрасываю биты, на место которых буду писать
        pAccessor.value = uint32_t(pAccessor.value) + uint32_t(sampleResampled);

        mDestinationBufferIt += size_bytes<OUTPUT_FORMAT>();
    }


    size_t remainingSampleCount() const {
        return (mDestinationBufferEnd - mDestinationBufferIt) / size_bytes<OUTPUT_FORMAT>();
    }

    template<SampleFormat f>
    inline void commitAllSamples(const _<ISoundStream>& is) {
        char buf[0x1000 * 3];
        for (size_t r; (r = is->read(buf, std::min(size_t(remainingSampleCount() * size_bytes<f>()), sizeof(buf)))) && remainingSampleCount() > 0;) {
            char* end = buf + r;
            for (char* it = buf; it + size_bytes<f>() <= end; it += size_bytes<f>()) {
                auto value = *reinterpret_cast<packed_accessor<f>*>(it);
                commitSample<f>(value.value);
            }
        }
    }

    bool isCommitted() const {
        return mDestinationBufferBegin != mDestinationBufferIt;
    }
    size_t writtenSize() const {
        return mDestinationBufferIt - mDestinationBufferBegin;
    }
};

class ISoundSource {
public:
    /**
     * Насрать в буфер звуковыми данными.
     * Срать нужно правильно! В соответствии с форматом фреймов. Используй класс SampleConsumer.
     * Не передаю его через ссылку, т.к. надеюсь, что компилятор хоть как-то оптимизирует это дерьмо
     */
    virtual size_t requestSoundData(char* dst, size_t size) = 0;
};


class SampleCommitter : public ISoundSource {
public:
    explicit SampleCommitter(_<ISoundStream> stream) : mSoundStream(std::move(stream)) {
        mFormat = mSoundStream->info();
    }

    size_t requestSoundData(char* dst, size_t size) override {
        SampleConsumer consumer(dst, dst + size);
        switch (mFormat.bitsPerSample) {
            case 16:
                consumer.commitAllSamples<SampleFormat::I16>(mSoundStream);
                break;
            case 24:
                consumer.commitAllSamples<SampleFormat::I24>(mSoundStream);
                break;
        }
        return consumer.writtenSize();
    }

private:
    _<ISoundStream> mSoundStream;
    AAudioFormat mFormat;

};

class Loop: public ISoundSource {
private:
    std::mutex mMutex;
    std::list<_<ISoundSource>> mSoundSources;

public:
    void addSoundSource(_<ISoundSource> s) {
        std::unique_lock lock(mMutex);
        mSoundSources.push_back(std::move(s));
    }
    void removeSoundSource(const _<ISoundSource>& s) {
        std::unique_lock lock(mMutex);
        mSoundSources.erase(std::remove(mSoundSources.begin(),
                                        mSoundSources.end(),
                                        s), mSoundSources.end());
    }
    size_t requestSoundData(char* dst, size_t size) override {
        std::memset(dst, 0, size);
        std::unique_lock lock(mMutex);
        std::list<_<ISoundSource>> itemsToRemove;
        size_t result = 0;
        for (auto& source : mSoundSources) {
            size_t r = source->requestSoundData(dst, size);
            if (r == 0) {
                itemsToRemove.push_back(source);
            } else {
                result = std::max(r, result);
            }
        }
        for (auto& i : itemsToRemove) {
            mSoundSources.erase(std::remove(mSoundSources.begin(),
                                            mSoundSources.end(),
                                            i), mSoundSources.end());
        }
        return size;
    }
};

