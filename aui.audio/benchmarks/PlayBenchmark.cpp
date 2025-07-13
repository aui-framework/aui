#include "AUI/Audio/IAudioPlayer.h"

#include <benchmark/benchmark.h>
#include "AUI/Common/AString.h"
#include "AUI/Thread/AEventLoop.h"
#include "AUI/Thread/IEventLoop.h"
#include "AUI/Traits/macros.h"

namespace {

class GeneratedSoundStream: public ISoundInputStream {
public:
    size_t read(char* dst, size_t size) override {
        auto span = std::span(reinterpret_cast<int16_t*>(dst), size / sizeof(int16_t));
        for (auto& s : span) {
            s = std::sin(mTime += 0.03f) * 1000;
        }
        mTime = std::fmod(mTime, glm::pi<float>() * 2.f);
        return span.size_bytes();
    }

    AAudioFormat info() override {
        return AAudioFormat {
            .channelCount = AChannelFormat::STEREO,
            .sampleRate = 44100,
            .sampleFormat = ASampleFormat::I16,
        };
    }

private:
    float mTime = 0;
};

}

static void Play(benchmark::State& state) {
    for (auto _ : state) {
        auto player = IAudioPlayer::fromStream(_new<GeneratedSoundStream>());
        player->play();
    }
}

BENCHMARK(Play);