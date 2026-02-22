// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <benchmark/benchmark.h>
#include "AUI/Util/SmallPimpl.h"
#include "AUI/Util/Assert.h"
#include <memory>
#include <functional>

// /home/alex2772/CLionProjects/aui/cmake-build-relwithdebinfo/bin/Benchmarks --benchmark_filter=.*
// 2026-02-20T02:56:27+03:00
// Running /home/alex2772/CLionProjects/aui/cmake-build-relwithdebinfo/bin/Benchmarks
// Run on (32 X 5086.18 MHz CPU s)
// CPU Caches:
//   L1 Data 32 KiB (x16)
//   L1 Instruction 32 KiB (x16)
//   L2 Unified 512 KiB (x16)
//   L3 Unified 32768 KiB (x2)
// Load Average: 14.71, 6.43, 3.65
// ***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
// ***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
// ----------------------------------------------------------------------------------------------------------
// Benchmark                                                                Time             CPU   Iterations
// ----------------------------------------------------------------------------------------------------------
// SmallPimpl_Allocation<aui::small_pimpl<ITest, 128>, SmallImpl>        2.12 ns         2.11 ns    333858259
// SmallPimpl_Allocation<std::function<void()>, SmallImpl>               8.59 ns         8.55 ns     83463874
// SmallPimpl_Allocation<aui::small_pimpl<ITest, 128>, MediumImpl>       3.22 ns         3.20 ns    220340695
// SmallPimpl_Allocation<std::function<void()>, MediumImpl>              8.36 ns         8.33 ns     82877739
// SmallPimpl_Allocation<aui::small_pimpl<ITest, 128>, BigImpl>          51.1 ns         50.7 ns     14034323
// SmallPimpl_Allocation<std::function<void()>, BigImpl>                 52.6 ns         52.4 ns     12969600
// SmallPimpl_Copy<aui::small_pimpl<ITest, 128>, SmallImpl>              4.24 ns         4.20 ns    166740471
// SmallPimpl_Copy<std::function<void()>, SmallImpl>                     10.2 ns         10.1 ns     70236187
// SmallPimpl_Copy<aui::small_pimpl<ITest, 128>, MediumImpl>             4.22 ns         4.20 ns    166158461
// SmallPimpl_Copy<std::function<void()>, MediumImpl>                    10.7 ns         10.7 ns     69750309
// SmallPimpl_Copy<aui::small_pimpl<ITest, 128>, BigImpl>                26.6 ns         26.5 ns     26582109
// SmallPimpl_Copy<std::function<void()>, BigImpl>                       26.0 ns         25.9 ns     27228219
// SmallPimpl_Invoke<aui::small_pimpl<ITest, 128>, SmallImpl>            1.16 ns         1.15 ns    609427092
// SmallPimpl_Invoke<std::function<void()>, SmallImpl>                   1.20 ns         1.19 ns    598789144
// SmallPimpl_Invoke<aui::small_pimpl<ITest, 128>, MediumImpl>           1.15 ns         1.15 ns    607650646
// SmallPimpl_Invoke<std::function<void()>, MediumImpl>                  1.39 ns         1.38 ns    508979367
// SmallPimpl_Invoke<aui::small_pimpl<ITest, 128>, BigImpl>              1.44 ns         1.42 ns    511851310
// SmallPimpl_Invoke<std::function<void()>, BigImpl>                     1.17 ns         1.17 ns    597973846

using namespace std::chrono_literals;

namespace {
struct ITest {
    virtual ~ITest() = default;
    virtual void test() = 0;

    void operator()() {
        this->test();
    }
};

// Small implementation (fits in stack buffer)
struct SmallImpl final: ITest {
    void test() override {}
};

// Large implementation (forces heap allocation for std::function only)
struct MediumImpl final: ITest {
    char padding[64];
    void test() override {}
};

// Large implementation (forces heap allocation)
struct BigImpl final: ITest {
    char padding[1024];
    void test() override {}
};
}

template<typename Pimpl, typename Impl>
static void SmallPimpl_Allocation(benchmark::State& state) {
    for (auto _ : state) {
        Pimpl pimpl{Impl{}};
        benchmark::DoNotOptimize(pimpl);
    }
}

template<typename Pimpl, typename Impl>
static void SmallPimpl_Copy(benchmark::State& state) {
    Pimpl pimpl1{Impl{}};
    for (auto _ : state) {
        Pimpl pimpl2{pimpl1};
        benchmark::DoNotOptimize(pimpl2);
    }
}

template<typename Pimpl, typename Impl>
static void SmallPimpl_Move(benchmark::State& state) {
    Pimpl pimpl1{Impl{}};
    for (auto _ : state) {
        state.PauseTiming();
        Pimpl pimpl2{pimpl1};
        state.ResumeTiming();
        Pimpl pimpl3{std::move(pimpl2)};
        benchmark::DoNotOptimize(pimpl3);
    }
}

void invoke(std::function<void()>& t) {
    std::invoke(t);
}

void invoke(aui::small_pimpl<ITest, 128>& t) {
    t->test();
}

template<typename Pimpl, typename Impl>
static void SmallPimpl_Invoke(benchmark::State& state) {
    Pimpl pimpl1{Impl{}};
    for (auto _ : state) {
        invoke(pimpl1);
    }
}


// std::function<void()> was added for comparison
BENCHMARK(SmallPimpl_Allocation<aui::small_pimpl<ITest, 128>, SmallImpl>);
BENCHMARK(SmallPimpl_Allocation<std::function<void()>, SmallImpl>);
BENCHMARK(SmallPimpl_Allocation<aui::small_pimpl<ITest, 128>, MediumImpl>);
BENCHMARK(SmallPimpl_Allocation<std::function<void()>, MediumImpl>);
BENCHMARK(SmallPimpl_Allocation<aui::small_pimpl<ITest, 128>, BigImpl>);
BENCHMARK(SmallPimpl_Allocation<std::function<void()>, BigImpl>);
BENCHMARK(SmallPimpl_Copy<aui::small_pimpl<ITest, 128>, SmallImpl>);
BENCHMARK(SmallPimpl_Copy<std::function<void()>, SmallImpl>);
BENCHMARK(SmallPimpl_Copy<aui::small_pimpl<ITest, 128>, MediumImpl>);
BENCHMARK(SmallPimpl_Copy<std::function<void()>, MediumImpl>);
BENCHMARK(SmallPimpl_Copy<aui::small_pimpl<ITest, 128>, BigImpl>);
BENCHMARK(SmallPimpl_Copy<std::function<void()>, BigImpl>);
BENCHMARK(SmallPimpl_Move<aui::small_pimpl<ITest, 128>, SmallImpl>);
BENCHMARK(SmallPimpl_Move<std::function<void()>, SmallImpl>);
BENCHMARK(SmallPimpl_Move<aui::small_pimpl<ITest, 128>, MediumImpl>);
BENCHMARK(SmallPimpl_Move<std::function<void()>, MediumImpl>);
BENCHMARK(SmallPimpl_Move<aui::small_pimpl<ITest, 128>, BigImpl>);
BENCHMARK(SmallPimpl_Move<std::function<void()>, BigImpl>);
BENCHMARK(SmallPimpl_Invoke<aui::small_pimpl<ITest, 128>, SmallImpl>);
BENCHMARK(SmallPimpl_Invoke<std::function<void()>, SmallImpl>);
BENCHMARK(SmallPimpl_Invoke<aui::small_pimpl<ITest, 128>, MediumImpl>);
BENCHMARK(SmallPimpl_Invoke<std::function<void()>, MediumImpl>);
BENCHMARK(SmallPimpl_Invoke<aui::small_pimpl<ITest, 128>, BigImpl>);
BENCHMARK(SmallPimpl_Invoke<std::function<void()>, BigImpl>);
