#include <benchmark/benchmark.h>
#include <AUI/Common/AObject.h>
#include "AUI/Common/ASignal.h"
#include "AUI/Util/Assert.h"
#include "AUI/Util/kAUI.h"

using namespace std::chrono_literals;

static constexpr auto TEST_VALUE = 228;

class Emitter: public AObject {
public:
    void makeSignal() {
        emit test(TEST_VALUE);
    }

signals:
    emits<int> test;
};

class Receiver: public AObject {
public:
    void receive(int v) {
        AUI_ASSERT(v == TEST_VALUE);
        benchmark::DoNotOptimize(v);
    }
};


static void SignalSlot(benchmark::State& state) {
    auto emitter = _new<Emitter>();
    auto receiver = _new<Receiver>();
    AObject::connect(emitter->test, AUI_SLOT(receiver)::receive);

    for (auto _ : state) {
        emitter->makeSignal();
    }
}
BENCHMARK(SignalSlot);
