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
#include <AUI/Util/kAUI.h>

#include <emscripten.h>

#include <cstdlib>
#include <unordered_map>

namespace {

struct Registry {
    std::unordered_map<int, aui::js::detail::Pending> pending;
    int lastId = 0;
};

Registry& registry() {
    static Registry r;
    return r;
}

AOptional<aui::js::detail::Pending> take(int id) {
    auto& pending = registry().pending;
    auto it = pending.find(id);
    if (it == pending.end()) {
        return std::nullopt;
    }
    auto result = std::move(it->second);
    pending.erase(it);
    return result;
}

// clang-format off
EM_JS(void, aui_js_promise_install, (), {
    if (Module.auiSettle) return;
    const aborts = new Map();
    const text = new TextEncoder();

    // the value in the wire format of aui::js::wire (AUI/Js/Value.h), in a block from aui_js_malloc: [address, size]
    const encode = (root) => {
        const ops = [];
        const blocks = [];   // the bytes copied into the heap: adopted by C++, freed here if encoding fails
        let size = 0;
        const visit = (v) => {
            if (v === undefined || v === null) { ops.push([0]); size += 1; return; }
            if (typeof v === 'boolean') { ops.push([v ? 2 : 1]); size += 1; return; }
            if (typeof v === 'number' || typeof v === 'bigint') { ops.push([3, Number(v)]); size += 9; return; }
            if (typeof v === 'string') { const b = text.encode(v); ops.push([4, b]); size += 5 + b.length; return; }
            if (v instanceof ArrayBuffer || ArrayBuffer.isView(v)) {
                const bytes = v instanceof ArrayBuffer ? new Uint8Array(v) : new Uint8Array(v.buffer, v.byteOffset, v.byteLength);
                let address = 0;
                if (bytes.length) {
                    address = _aui_js_malloc(bytes.length);
                    if (!address) throw new Error('aui::js: out of memory for ' + bytes.length + ' bytes');
                    blocks.push(address);
                    HEAPU8.set(bytes, address);
                }
                ops.push([5, address, bytes.length]);
                size += 17;
                return;
            }
            if (Array.isArray(v)) { ops.push([6, v.length]); size += 5; v.forEach(visit); return; }
            if (typeof v === 'object') {
                const entries = Object.entries(v);
                ops.push([7, entries.length]);
                size += 5;
                for (const [key, item] of entries) {
                    const b = text.encode(key);
                    ops.push([8, b]);
                    size += 4 + b.length;
                    visit(item);
                }
                return;
            }
            throw new Error('aui::js: a ' + typeof v + ' can not be passed to C++');
        };
        let address = 0;
        try {
            visit(root);
            address = _aui_js_malloc(size);
            if (!address) throw new Error('aui::js: out of memory for ' + size + ' bytes');
        } catch (error) {
            blocks.forEach((b) => _aui_js_free(b));
            throw error;
        }
        // after the last allocation: the heap may have grown, HEAPU8 is its current view
        const view = new DataView(HEAPU8.buffer, address, size);
        let at = 0;
        const u32 = (n) => { view.setUint32(at, n, true); at += 4; };
        const u64 = (n) => { u32(n); u32(Math.floor(n / 4294967296)); };
        const utf8 = (b) => { u32(b.length); HEAPU8.set(b, address + at); at += b.length; };
        for (const op of ops) {
            if (op[0] !== 8) view.setUint8(at++, op[0]);
            switch (op[0]) {
                case 3: view.setFloat64(at, op[1], true); at += 8; break;
                case 4: case 8: utf8(op[1]); break;
                case 5: u64(op[1]); u64(op[2]); break;
                case 6: case 7: u32(op[1]); break;
            }
        }
        return [address, size];
    };

    const reject = (id, reason) => {
        const [address, size] = encode(String((reason && reason.stack) || reason));
        _aui_js_promise_reject(id, address, size);
    };

    // settles the AFuture `id` of aui::js::promise with `work`: a promise (or a value), or a function that takes an
    // AbortSignal, aborted when the future is cancelled, and returns one
    Module.auiSettle = (id, work) => {
        let promise;
        if (typeof work === 'function') {
            const controller = new AbortController();
            aborts.set(id, controller);
            try {
                promise = Promise.resolve(work(controller.signal));
            } catch (error) {
                promise = Promise.reject(error);
            }
        } else {
            promise = Promise.resolve(work);
        }
        promise.then((value) => {
            aborts.delete(id);
            let block;
            try {
                block = encode(value);
            } catch (error) {
                reject(id, error);
                return;
            }
            _aui_js_promise_resolve(id, block[0], block[1]);
        }, (reason) => {
            aborts.delete(id);
            reject(id, reason);
        });
    };
    Module.auiAbort = (id) => {
        const controller = aborts.get(id);
        aborts.delete(id);
        if (controller) controller.abort();
    };
});

EM_JS(void, aui_js_promise_abort, (int id), {
    Module.auiAbort(id);
});
// clang-format on

}   // namespace

// the heap for JavaScript: exported by itself, no matter what the application exports
extern "C" EMSCRIPTEN_KEEPALIVE void* aui_js_malloc(std::size_t size) { return std::malloc(size); }
extern "C" EMSCRIPTEN_KEEPALIVE void aui_js_free(void* block) { std::free(block); }

/// `block` comes from `aui_js_malloc`: freed here, the byte buffers in it are adopted, even if nobody waits anymore.
extern "C" EMSCRIPTEN_KEEPALIVE void aui_js_promise_resolve(int id, std::byte* block, std::size_t size) {
    AUI_DEFER { std::free(block); };
    auto pending = take(id);
    try {
        auto value = aui::js::wire::decode({ block, size });
        if (pending) pending->resolve(std::move(value));
    } catch (...) {
        if (pending) pending->reject(std::current_exception());
    }
}

/// `block` is the reason as a string in the wire format, from `aui_js_malloc`: freed here.
extern "C" EMSCRIPTEN_KEEPALIVE void aui_js_promise_reject(int id, std::byte* block, std::size_t size) {
    AUI_DEFER { std::free(block); };
    auto reason = aui::js::wire::decode({ block, size });
    auto* text = std::get_if<AString>(&reason);
    if (auto pending = take(id)) {
        pending->reject(std::make_exception_ptr(
            AException("JavaScript promise rejected: " + (text ? *text : AString(aui::js::typeName(reason))))));
    }
}

int aui::js::detail::adopt(Pending pending) {
    aui_js_promise_install();
    auto& r = registry();
    const int id = ++r.lastId;
    r.pending.emplace(id, std::move(pending));
    return id;
}

void aui::js::detail::forget(int id) { registry().pending.erase(id); }

void aui::js::detail::abort(int id) { aui_js_promise_abort(id); }
