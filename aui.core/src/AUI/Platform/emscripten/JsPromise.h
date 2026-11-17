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

#include <AUI/Js/Value.h>
#include <AUI/Thread/AFuture.h>

#include <concepts>
#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

namespace aui::js {

namespace detail {
struct Pending {
    std::function<void(Value)> resolve;
    std::function<void(std::exception_ptr)> reject;
};

/// Keeps the callbacks until JavaScript settles the promise; returns the id JavaScript refers to it by.
API_AUI_CORE int adopt(Pending pending);
/// Drops the callbacks of `id` without a call.
API_AUI_CORE void forget(int id);
/// Aborts the AbortSignal JavaScript got for `id`, if it took one.
API_AUI_CORE void abort(int id);
}   // namespace detail

/**
 * @brief A JavaScript promise as an AFuture.
 * @details
 * `start` gets the id of the future and starts the JavaScript side, usually an `EM_JS` function that passes
 * `Module.auiSettle(id, work)` either a promise or a function that takes an AbortSignal and returns a promise (or a
 * value). The promise resolves with any value of JSON plus binary data (`ArrayBuffer`, typed arrays), which arrives as
 * aui::js::Value and is converted to T by aui::js::Converter. If it is something else, or the promise rejects, the
 * future fails with AException that says why.
 *
 * ```cpp
 * EM_JS(void, fetchBytes, (int id, const char* url), {
 *     const u = UTF8ToString(url);
 *     Module.auiSettle(id, (signal) => fetch(u, { signal }).then((r) => r.arrayBuffer()));
 * });
 *
 * AFuture<AByteBuffer> download(AString url) {
 *     const auto u = url.toStdString();
 *     return aui::js::promise<AByteBuffer>([&](int id) { fetchBytes(id, u.c_str()); });
 * }
 * ```
 *
 * Several values come as an array: `promise<std::tuple<std::uint32_t, std::uint32_t, AByteBuffer>>` for
 * `[width, height, pixels]`. `promise<void>` ignores the value; `promise<>` gives Value as it is.
 *
 * Binary data is copied into the wasm heap once (`malloc` called from JavaScript) and the AByteBuffer adopts that
 * block, so a big buffer is not copied again. Whatever JavaScript allocated is freed by C++, even if nobody waits for the
 * value anymore.
 *
 * When the last AFuture is destroyed, the future is cancelled and the AbortSignal (if `work` took one) is aborted:
 * `fetch` stops, a worker job can be dropped. The promise still settles later, its result is freed and ignored.
 *
 * The future is settled on the main thread, from a promise reaction, so its callbacks and the coroutines awaiting it
 * continue there. The browser can settle the promise only after the control returns to it: do not block on the
 * future (`operator*`, `get()`, `wait()`), await it or subscribe to it.
 */
template <typename T = Value, std::invocable<int> Start>
[[nodiscard]]
AFuture<T> promise(Start&& start) {
    return AFuture<T>::create([&](typename AFuture<T>::Resolver resolver) {
        const int id = detail::adopt({
            .resolve =
                [resolver](Value value) {
                    try {
                        if constexpr (std::is_void_v<T>) {
                            resolver.supplyValue();
                        } else {
                            resolver.supplyValue(from<T>(std::move(value)));
                        }
                    } catch (...) {
                        resolver.supplyException();
                    }
                },
            .reject = [resolver](std::exception_ptr e) { resolver.supplyException(std::move(e)); },
        });
        resolver.onCancel([id] { detail::abort(id); });
        try {
            std::forward<Start>(start)(id);
        } catch (...) {
            detail::forget(id);
            throw;
        }
    });
}

}   // namespace aui::js
