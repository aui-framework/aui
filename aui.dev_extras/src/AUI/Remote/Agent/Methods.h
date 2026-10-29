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

#include <functional>
#include <mutex>

#include "JsonRpc.h"
#include "RefTable.h"

namespace aui::remote {

/**
 * @brief Looks up a window by the optional `windowId` param; without it, the first window that is not a popup.
 * @throws RpcException with NO_SUCH_WINDOW or STALE_REF
 */
_<AWindow> windowFromParams(const AJson& params, RefTable& refs);

/**
 * @brief The implementation of `view.find`; must be called on the UI thread.
 * @return array of described views
 */
AJson findViews(RefTable& refs, const AJson& params);

/**
 * `window.list`, `view.tree`, `view.find`, `view.properties`, `view.hitTest`, `view.interactive`.
 */
void registerInspectionMethods(JsonRpcDispatcher& dispatcher, RefTable& refs);

/**
 * `window.resize`.
 */
void registerWindowMethods(JsonRpcDispatcher& dispatcher, RefTable& refs);

/**
 * `screenshot`.
 */
void registerScreenshotMethods(JsonRpcDispatcher& dispatcher, RefTable& refs);

/**
 * @brief The pointer `input.pointer` uses when the client does not choose one: `finger` on mobile platforms, `mouse`
 * elsewhere.
 */
AString defaultPointer();

/**
 * `input.pointer`, `input.type`, `input.key`.
 */
void registerInputMethods(JsonRpcDispatcher& dispatcher, RefTable& refs);

/**
 * `wait.idle`, `wait.view`.
 */
void registerWaitMethods(JsonRpcDispatcher& dispatcher, RefTable& refs);

/**
 * `frames.subscribe`, `frames.unsubscribe`, `frames.ack`: the window as a stream of JPEG frames along with the diffs of
 * its view tree, pushed as `frames.frame` notifications. See docs/agentic-development.md.
 */
void registerFrameMethods(JsonRpcDispatcher& dispatcher, RefTable& refs);

/**
 * @brief Functions the application exposes to the client with ARemoteDebug::registerFunction.
 */
class FunctionRegistry {
public:
    using Handler = std::function<AFuture<AJson>(const AJson& arguments)>;

    void add(AString name, AString description, Handler handler);

    struct Entry {
        AString description;
        Handler handler;
    };
    [[nodiscard]] AOptional<Entry> find(const AString& name) const;
    [[nodiscard]] AMap<AString, AString> descriptions() const;

private:
    mutable std::mutex mMutex;
    AMap<AString, Entry> mEntries;
};

/**
 * `fn.list`, `fn.call`.
 */
void registerFunctionMethods(JsonRpcDispatcher& dispatcher, FunctionRegistry& functions);

/**
 * `diag.log`: the recent log entries.
 */
void registerDiagnosticsMethods(JsonRpcDispatcher& dispatcher);

}   // namespace aui::remote
