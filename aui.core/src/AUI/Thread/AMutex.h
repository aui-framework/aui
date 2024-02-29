// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
