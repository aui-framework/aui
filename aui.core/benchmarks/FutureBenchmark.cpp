#include <benchmark/benchmark.h>
#include "AUI/Thread/AFuture.h"
#include "AUI/Thread/AThreadPool.h"
#include "AUI/Util/Assert.h"


static constexpr auto VALUE = 228;

static void FutureImmediateValue(benchmark::State& state) {
    for (auto _ : state) {
        AFuture f(VALUE);
        auto value = *f;
        AUI_ASSERT(value == VALUE);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(FutureImmediateValue);

static void FutureSingleThread(benchmark::State& state) {
    for (auto _ : state) {
        AFuture<int> f;
        f.supplyValue(VALUE);
        auto value = *f;
        AUI_ASSERT(value == VALUE);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(FutureSingleThread);

static void FutureMultiThread(benchmark::State& state) {
    AThreadPool tp(1);
    for (auto _ : state) {
        AFuture<int> f = tp * [] {
            return VALUE;  
        };
        auto value = *f;
        AUI_ASSERT(value == VALUE);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(FutureMultiThread);