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

#include "AUI/Common/AException.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/AString.h"
#include "AUI/Json/AJson.h"
#include "AUI/Thread/AFuture.h"
#include "AUI/api.h"

namespace aui::remote {

/**
 * @brief JSON-RPC 2.0 error codes: the ones of the specification and the ones of the remote agent.
 */
namespace rpc_code {
constexpr int PARSE_ERROR = -32700;
constexpr int INVALID_REQUEST = -32600;
constexpr int METHOD_NOT_FOUND = -32601;
constexpr int INVALID_PARAMS = -32602;
constexpr int INTERNAL_ERROR = -32603;

constexpr int UNAUTHORIZED = -32001;
constexpr int STALE_REF = -32002;
constexpr int NO_SUCH_WINDOW = -32003;
constexpr int NOT_SUPPORTED = -32004;
constexpr int TIMEOUT = -32005;
constexpr int UNKNOWN_FUNCTION = -32006;
}   // namespace rpc_code

/**
 * @brief Thrown by method handlers to answer with a JSON-RPC error.
 */
class API_AUI_DEV_EXTRAS RpcException : public AException {
public:
    RpcException(int code, AString message, AString hint = {});

    [[nodiscard]] int code() const noexcept { return mCode; }
    [[nodiscard]] const AString& hint() const noexcept { return mHint; }

private:
    int mCode;
    AString mHint;
};

/**
 * @brief Routes JSON-RPC 2.0 messages to handlers.
 * @details
 * Handlers return a future, so a method can complete later (i.e., `wait.idle`). A handler reports an error by throwing
 * RpcException (or supplying it to the future); a JSON type mismatch while reading params turns into "invalid params",
 * any other exception into "internal error".
 *
 * Notifications (requests without `id`) are executed and not answered. Batches are supported.
 *
 * The dispatcher does not care about threads: `handle` may be called from any thread, handlers and `respond` are
 * called on whatever thread executes them.
 */
/**
 * @brief Builds a JSON-RPC error response.
 */
API_AUI_DEV_EXTRAS AJson jsonRpcError(const AJson& id, int code, const AString& message, const AString& hint = {});

class API_AUI_DEV_EXTRAS JsonRpcDispatcher {
public:
    using Handler = std::function<AFuture<AJson>(const AJson& params)>;
    using SyncHandler = std::function<AJson(const AJson& params)>;
    using Respond = std::function<void(AString response)>;

    void registerMethod(AString name, Handler handler);
    void registerMethod(AString name, SyncHandler handler);

    [[nodiscard]] bool hasMethod(const AString& name) const;

    /**
     * @return names of the registered methods, sorted
     */
    [[nodiscard]] AVector<AString> methodNames() const;

    /**
     * @brief Processes a single message (a request or a batch).
     * @param respond called with the serialized response, unless there is nothing to answer.
     */
    void handle(const AString& message, Respond respond) const;

private:
    using Reply = std::function<void(AOptional<AJson>)>;
    void handleOne(const AJson& request, Reply reply) const;

    mutable std::mutex mMutex;
    AMap<AString, Handler> mMethods;
};

}   // namespace aui::remote
