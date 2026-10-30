/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <exception>
#include <memory>
#include <type_traits>

#include "AUI/Thread/AFuture.h"
#include "AUI/Thread/AThread.h"

namespace aui::remote {

/**
 * @brief Runs `fn` on the UI thread and returns the future of its result.
 * @details
 * Never blocks the caller and never runs `fn` inline, even when called from the UI thread. An exception thrown by `fn`
 * ends up in the future.
 *
 * Do not wait for the returned future on the UI thread.
 */
template <typename Fn>
auto onUiThread(Fn fn) {
    using Result = std::invoke_result_t<Fn&>;
    return AFuture<Result>::create([&](typename AFuture<Result>::Resolver resolver) {
        // std::function requires a copyable callable
        AThread::main()->enqueue([shared = std::make_shared<Fn>(std::move(fn)), resolver = std::move(resolver)] {
            if (resolver.isCancelled()) {
                return;
            }
            try {
                if constexpr (std::is_void_v<Result>) {
                    (*shared)();
                    resolver.supplyValue();
                } else {
                    resolver.supplyValue((*shared)());
                }
            } catch (...) {
                resolver.supplyException();
            }
        });
    });
}

}   // namespace aui::remote
