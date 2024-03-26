#include <benchmark/benchmark.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Platform/AProcess.h>
#include "AUI/Json/AJson.h"
#include "AUI/Thread/AThread.h"

static void JsonParse(benchmark::State& state) {
    auto rawJson = ACurl::Builder("https://raw.githubusercontent.com/json-iterator/test-data/master/large-file.json").runBlocking().body;
    using namespace std::chrono_literals;

    auto beforeTest = AProcess::self()->processMemory();
    size_t memUsage = 0;
    for (auto _ : state) {
        auto json = AJson::fromBuffer(rawJson);
        benchmark::DoNotOptimize(json);
        auto memDiff = AProcess::self()->processMemory() - beforeTest;
        memUsage = std::max(memDiff, memUsage);
    }

    state.counters["ProcessMemory"] = memUsage;
}

BENCHMARK(JsonParse)->Iterations(10);