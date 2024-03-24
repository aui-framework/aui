#include <benchmark/benchmark.h>
#include "AUI/Common/AString.h"
#include "AUI/Traits/macros.h"


static void StringUTF8(benchmark::State& state) {
    for (auto _ : state) {
        AString s = "ололо";
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK(StringUTF8);