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
        auto f = AFuture<int>::fromValue(VALUE);
        auto value = *f;
        AUI_ASSERT(value == VALUE);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(FutureImmediateValue);

static void FutureSingleThread(benchmark::State& state) {
    for (auto _ : state) {
        auto f = AFuture<int>::create([](AFuture<int>::Resolver r) { r.supplyValue(VALUE); });
        auto value = *f;
        AUI_ASSERT(value == VALUE);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(FutureSingleThread);

/// A copy of a future: what passing it around costs.
static void FutureCopy(benchmark::State& state) {
    auto f = AFuture<int>::fromValue(VALUE);
    for (auto _ : state) {
        auto copy = f;
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(FutureCopy);

/// A callback is subscribed before the result arrives.
static void FutureSubscribeThenResolve(benchmark::State& state) {
    for (auto _ : state) {
        AOptional<AFuture<int>::Resolver> resolver;
        auto f = AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); });
        int received = 0;
        f.onValue([&](int v) { received = v; });
        resolver->supplyValue(VALUE);
        AUI_ASSERT(received == VALUE);
        benchmark::DoNotOptimize(received);
    }
}
BENCHMARK(FutureSubscribeThenResolve);

/// A pending future nobody waits for anymore: cancellation by destruction.
static void FutureDropPending(benchmark::State& state) {
    for (auto _ : state) {
        AOptional<AFuture<int>::Resolver> resolver;
        auto f = AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); });
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(FutureDropPending);

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

/// 100 tasks at once on a pool of 4.
static void FutureThreadPoolBatch(benchmark::State& state) {
    AThreadPool tp(4);
    for (auto _ : state) {
        AVector<AFuture<int>> futures;
        futures.reserve(100);
        AUI_REPEAT(100) {
            futures << tp * [] { return VALUE; };
        }
        int sum = 0;
        for (auto& f : futures) {
            sum += *f;
        }
        AUI_ASSERT(sum == VALUE * 100);
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(FutureThreadPoolBatch);

#if AUI_COROUTINES
static AFuture<int> coroutineChain(int depth) {
    if (depth == 0) {
        co_return VALUE;
    }
    co_return co_await coroutineChain(depth - 1);
}

/// 10 nested coroutines, each awaits a future that is already resolved.
static void FutureCoroutineChain(benchmark::State& state) {
    for (auto _ : state) {
        auto value = *coroutineChain(10);
        AUI_ASSERT(value == VALUE);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(FutureCoroutineChain);
#endif
