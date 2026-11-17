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

#include "JsPromise.h"

#include <AUI/Common/AException.h>

#include <emscripten.h>

#include <cstdlib>
#include <unordered_map>

namespace {

struct Pending {
    std::unordered_map<int, AFuture<double>::Resolver> resolvers;
    int lastId = 0;
};

Pending& pending() {
    static Pending p;
    return p;
}

AOptional<AFuture<double>::Resolver> take(int id) {
    auto& resolvers = pending().resolvers;
    auto it = resolvers.find(id);
    if (it == resolvers.end()) {
        return std::nullopt;
    }
    auto resolver = std::move(it->second);
    resolvers.erase(it);
    return resolver;
}

// clang-format off
EM_JS_DEPS(aui_js_promise, "$stringToNewUTF8");

EM_JS(void, aui_js_promise_install, (), {
    if (Module.auiSettle) return;
    // settles the AFuture `id` of aui::js::promise with `promise`: its value as a number, or the text of the rejection
    Module.auiSettle = (id, promise) => Promise.resolve(promise).then(
        (value) => _aui_js_promise_resolve(id, Number(value)),
        (reason) => _aui_js_promise_reject(id, stringToNewUTF8(String((reason && reason.stack) || reason))));
});
// clang-format on

}   // namespace

extern "C" EMSCRIPTEN_KEEPALIVE void aui_js_promise_resolve(int id, double value) {
    if (auto resolver = take(id)) {
        resolver->supplyValue(value);
    }
}

/// `reason` is allocated by stringToNewUTF8: freed here.
extern "C" EMSCRIPTEN_KEEPALIVE void aui_js_promise_reject(int id, char* reason) {
    AString text = reason;
    std::free(reason);
    if (auto resolver = take(id)) {
        resolver->supplyException(std::make_exception_ptr(AException("JavaScript promise rejected: " + text)));
    }
}

int aui::js::detail::adopt(AFuture<double>::Resolver resolver) {
    aui_js_promise_install();
    auto& p = pending();
    const int id = ++p.lastId;
    p.resolvers.emplace(id, std::move(resolver));
    return id;
}

void aui::js::detail::forget(int id) { pending().resolvers.erase(id); }
