/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <mutex>
#include <shared_mutex>
#include <thread>
#include "AUI/Performance/APerformanceSection.h"


namespace aui::detail {
    template<typename T>
    struct MutexExtras: T {
    public:
        void lock() {
            APerformanceSection section("Mutex", AColor::RED);
            T::lock();
        }
    };
}

struct AMutex: aui::detail::MutexExtras<std::mutex>{};
struct ARecursiveMutex: aui::detail::MutexExtras<std::recursive_mutex>{};
struct ASharedMutex: aui::detail::MutexExtras<std::shared_mutex> {
public:
    void lock_shared() {
        APerformanceSection section("Mutex", AColor::RED);
        MutexExtras::lock_shared();
    }
};
