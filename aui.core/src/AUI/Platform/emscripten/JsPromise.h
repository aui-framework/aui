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

#include <AUI/Thread/AFuture.h>

#include <concepts>
#include <utility>

namespace aui::js {

namespace detail {
/// Keeps the resolver until JavaScript settles it; returns the id JavaScript refers to it by.
API_AUI_CORE int adopt(AFuture<double>::Resolver resolver);
/// Drops the resolver of `id`: its future fails with AFutureAbandonedException.
API_AUI_CORE void forget(int id);
}   // namespace detail

/**
 * @brief A JavaScript promise as an AFuture.
 * @details
 * `start` gets the id of the future and starts the JavaScript side, usually an `EM_JS` function that hands its promise
 * to `Module.auiSettle(id, promise)`. The future gets the number the promise resolves with (integers and wasm
 * pointers fit into a double) or fails with AException carrying the rejection reason as text.
 *
 * ```cpp
 * EM_JS(void, fetchLength, (int id, const char* url), {
 *     Module.auiSettle(id, fetch(UTF8ToString(url)).then((r) => r.arrayBuffer()).then((b) => b.byteLength));
 * });
 *
 * AFuture<std::size_t> length(AString url) {
 *     const auto u = url.toStdString();
 *     co_return std::size_t(co_await aui::js::promise([&](int id) { fetchLength(id, u.c_str()); }));
 * }
 * ```
 *
 * The future is settled on the main thread, from the browser's event loop (a promise reaction), so its callbacks and
 * the coroutines awaiting it continue there. The browser can settle the promise only after the control returns to
 * it: do not block on the future (`operator*`, `get()`, `wait()`), await it or subscribe to it.
 *
 * Dropping the future does not stop the JavaScript work, its result is ignored. If the result is something to be freed
 * (a pointer from `_malloc`, an entry of a JavaScript table), keep the future until it settles.
 */
template <std::invocable<int> Start>
[[nodiscard]]
AFuture<double> promise(Start&& start) {
    return AFuture<double>::create([&](AFuture<double>::Resolver resolver) {
        const int id = detail::adopt(std::move(resolver));
        try {
            std::forward<Start>(start)(id);
        } catch (...) {
            detail::forget(id);
            throw;
        }
    });
}

}   // namespace aui::js
