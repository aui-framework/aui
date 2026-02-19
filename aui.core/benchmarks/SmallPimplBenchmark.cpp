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
BENCHMARK(SmallPimpl_Invoke<aui::small_pimpl<ITest, 128>, SmallImpl>);
BENCHMARK(SmallPimpl_Invoke<std::function<void()>, SmallImpl>);
BENCHMARK(SmallPimpl_Invoke<aui::small_pimpl<ITest, 128>, MediumImpl>);
BENCHMARK(SmallPimpl_Invoke<std::function<void()>, MediumImpl>);
BENCHMARK(SmallPimpl_Invoke<aui::small_pimpl<ITest, 128>, BigImpl>);
BENCHMARK(SmallPimpl_Invoke<std::function<void()>, BigImpl>);
