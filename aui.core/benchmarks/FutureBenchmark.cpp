#include <benchmark/benchmark.h>
#include "AUI/Thread/AFuture.h"
#include "AUI/Thread/AThread.h"
#include "AUI/Thread/AThreadPool.h"
#include "AUI/Util/Assert.h"
#include "AUI/Util/kAUI.h"

using namespace std::chrono_literals;

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

static void FutureMultiThread1(benchmark::State& state) {
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
BENCHMARK(FutureMultiThread1);

static void FutureMultiThread2(benchmark::State& state) {
    AThreadPool tp(1);
    for (auto _ : state) {
        AVector<AFuture<int>> futures;
        AUI_REPEAT(100) {
            futures << tp * [] {
                AThread::sleep(1ms);
                return VALUE;  
            };
        }
        auto value = *(tp * [] {
            return VALUE;
        });
        AUI_ASSERT(value == VALUE);
        for (auto& f : futures) {
            benchmark::DoNotOptimize(*f);
        }
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(FutureMultiThread2);