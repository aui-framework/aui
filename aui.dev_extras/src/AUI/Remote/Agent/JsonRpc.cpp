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

#include "JsonRpc.h"

#include <algorithm>

#include "AUI/Json/Exception.h"
#include "AUI/Logging/ALogger.h"

namespace aui::remote {

namespace {
constexpr auto LOG_TAG = "JsonRpc";

AJson errorResponse(const AJson& id, int code, const AString& message, const AString& hint = {}) {
    AJson error {
        { "code", code },
        { "message", message },
    };
    if (!hint.empty()) {
        error["data"] = AJson { { "hint", hint } };
    }
    return AJson {
        { "jsonrpc", "2.0" },
        { "id", id },
        { "error", std::move(error) },
    };
}

AJson successResponse(const AJson& id, AJson result) {
    return AJson {
        { "jsonrpc", "2.0" },
        { "id", id },
        { "result", std::move(result) },
    };
}

/// Turns whatever a handler threw into an error response.
AJson errorFromException(const AJson& id, std::exception_ptr e) {
    try {
        std::rethrow_exception(std::move(e));
    } catch (const RpcException& e) {
        return errorResponse(id, e.code(), e.getMessage(), e.hint());
    } catch (const AJsonException& e) {
        return errorResponse(id, rpc_code::INVALID_PARAMS, e.getMessage(), "check the types of the params");
    } catch (const AException& e) {
        // AFuture wraps exceptions; unwrap to the original one if there is any
        if (e.causedBy()) {
            return errorFromException(id, e.causedBy());
        }
        return errorResponse(id, rpc_code::INTERNAL_ERROR, e.getMessage());
    } catch (const std::exception& e) {
        return errorResponse(id, rpc_code::INTERNAL_ERROR, e.what());
    } catch (...) {
        return errorResponse(id, rpc_code::INTERNAL_ERROR, "unknown error");
    }
}
}   // namespace

RpcException::RpcException(int code, AString message, AString hint)
  : AException(std::move(message), nullptr), mCode(code), mHint(std::move(hint)) {}

AJson jsonRpcError(const AJson& id, int code, const AString& message, const AString& hint) {
    return errorResponse(id, code, message, hint);
}

RpcConnection::~RpcConnection() = default;

void RpcConnection::addCloseListener(std::function<void()> listener) {
    {
        std::lock_guard lock(mMutex);
        if (!mClosed) {
            mListeners.push_back(std::move(listener));
            return;
        }
    }
    listener();
}

void RpcConnection::notifyClosed() {
    AVector<std::function<void()>> listeners;
    {
        std::lock_guard lock(mMutex);
        if (mClosed) {
            return;
        }
        mClosed = true;
        listeners = std::move(mListeners);
    }
    for (const auto& listener : listeners) {
        listener();
    }
}

void JsonRpcDispatcher::registerConnectionMethod(AString name, ConnectionHandler handler) {
    std::lock_guard lock(mMutex);
    mMethods[std::move(name)] = std::move(handler);
}

void JsonRpcDispatcher::registerMethod(AString name, Handler handler) {
    registerConnectionMethod(
        std::move(name), ConnectionHandler([handler = std::move(handler)](const AJson& params, const _<RpcConnection>&) {
            return handler(params);
        }));
}

void JsonRpcDispatcher::registerMethod(AString name, SyncHandler handler) {
    registerMethod(std::move(name), Handler([handler = std::move(handler)](const AJson& params) {
        return AFuture<AJson>(handler(params));
    }));
}

bool JsonRpcDispatcher::hasMethod(const AString& name) const {
    std::lock_guard lock(mMutex);
    return mMethods.contains(name);
}

AVector<AString> JsonRpcDispatcher::methodNames() const {
    std::lock_guard lock(mMutex);
    AVector<AString> names;
    for (const auto& [name, _] : mMethods) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

void JsonRpcDispatcher::handle(const AString& message, Respond respond, _<RpcConnection> connection) const {
    AJson request;
    try {
        request = AJson::fromString(message);
    } catch (const AException& e) {
        respond(AJson::toString(errorResponse(nullptr, rpc_code::PARSE_ERROR, "parse error", e.getMessage())));
        return;
    }

    if (request.isArray()) {
        const auto& items = request.asArray();
        if (items.empty()) {
            respond(AJson::toString(errorResponse(nullptr, rpc_code::INVALID_REQUEST, "empty batch")));
            return;
        }
        struct Batch {
            std::mutex mutex;
            std::size_t pending;
            aui::impl::JsonArray responses;
            Respond respond;
        };
        auto batch = std::make_shared<Batch>();
        batch->pending = items.size();
        batch->respond = std::move(respond);
        for (const auto& item : items) {
            handleOne(item, [batch](AOptional<AJson> response) {
                std::unique_lock lock(batch->mutex);
                if (response) {
                    batch->responses.push_back(std::move(*response));
                }
                if (--batch->pending != 0) {
                    return;
                }
                auto responses = std::move(batch->responses);
                lock.unlock();
                if (!responses.empty()) {
                    batch->respond(AJson::toString(AJson(std::move(responses))));
                }
            }, connection);
        }
        return;
    }

    handleOne(request, [respond = std::move(respond)](AOptional<AJson> response) {
        if (response) {
            respond(AJson::toString(*response));
        }
    }, connection);
}

void JsonRpcDispatcher::handleOne(const AJson& request, Reply reply, const _<RpcConnection>& connection) const {
    AJson id = nullptr;
    if (!request.isObject()) {
        reply(errorResponse(id, rpc_code::INVALID_REQUEST, "request must be an object"));
        return;
    }

    const auto idField = request.containsOpt("id");
    if (idField) {
        if (!(idField->isString() || idField->isNumber() || idField->isNull())) {
            reply(errorResponse(id, rpc_code::INVALID_REQUEST, "id must be a string, a number or null"));
            return;
        }
        id = *idField;
    }
    // notifications are not answered
    auto answer = [reply, hasId = idField.hasValue()](AJson response) {
        if (hasId) {
            reply(std::move(response));
        } else {
            reply(std::nullopt);
        }
    };

    const auto version = request.containsOpt("jsonrpc");
    const auto methodField = request.containsOpt("method");
    if (!version || !version->isString() || version->asString() != "2.0" || !methodField || !methodField->isString()) {
        answer(errorResponse(id, rpc_code::INVALID_REQUEST, "not a JSON-RPC 2.0 request",
                             "expected {\"jsonrpc\":\"2.0\",\"method\":\"...\",\"id\":...}"));
        return;
    }

    AJson params = AJson::fromString("{}");
    if (const auto p = request.containsOpt("params")) {
        if (!p->isObject() && !p->isArray()) {
            answer(errorResponse(id, rpc_code::INVALID_PARAMS, "params must be an object or an array"));
            return;
        }
        params = *p;
    }

    ConnectionHandler handler;
    {
        std::lock_guard lock(mMutex);
        if (auto it = mMethods.find(methodField->asString()); it != mMethods.end()) {
            handler = it->second;
        }
    }
    if (!handler) {
        answer(errorResponse(id, rpc_code::METHOD_NOT_FOUND, "method not found: " + methodField->asString()));
        return;
    }

    AFuture<AJson> future;
    try {
        future = handler(params, connection);
    } catch (...) {
        answer(errorFromException(id, std::current_exception()));
        return;
    }
    future.onSuccess([answer, id](const AJson& result) { answer(successResponse(id, result)); });
    future.onError([answer, id](const AException& e) {
        answer(errorFromException(id, e.causedBy() ? e.causedBy() : std::make_exception_ptr(e)));
    });
}

}   // namespace aui::remote
