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

#include "Methods.h"

#include "AUI/Thread/AThread.h"

namespace aui::remote {

void FunctionRegistry::add(AString name, AString description, Handler handler) {
    std::lock_guard lock(mMutex);
    mEntries[std::move(name)] = Entry { std::move(description), std::move(handler) };
}

AOptional<FunctionRegistry::Entry> FunctionRegistry::find(const AString& name) const {
    std::lock_guard lock(mMutex);
    if (auto it = mEntries.find(name); it != mEntries.end()) {
        return it->second;
    }
    return std::nullopt;
}

AMap<AString, AString> FunctionRegistry::descriptions() const {
    std::lock_guard lock(mMutex);
    AMap<AString, AString> result;
    for (const auto& [name, entry] : mEntries) {
        result[name] = entry.description;
    }
    return result;
}

void registerFunctionMethods(JsonRpcDispatcher& dispatcher, FunctionRegistry& functions) {
    dispatcher.registerMethod("fn.list", JsonRpcDispatcher::SyncHandler([&functions](const AJson&) {
        aui::impl::JsonArray list;
        for (const auto& [name, description] : functions.descriptions()) {
            list.push_back(AJson {
              { "name", name },
              { "description", description },
            });
        }
        return AJson(std::move(list));
    }));

    // params: name, arguments? (anything the function understands)
    dispatcher.registerMethod("fn.call", JsonRpcDispatcher::Handler([&functions](const AJson& params) {
        if (!params.containsOpt("name")) {
            throw RpcException(rpc_code::INVALID_PARAMS, "missing param: name");
        }
        const auto name = params["name"].asString();
        auto entry = functions.find(name);
        if (!entry) {
            throw RpcException(rpc_code::UNKNOWN_FUNCTION, "no such function: " + name, "see fn.list");
        }
        AJson arguments = params.containsOpt("arguments") ? params["arguments"] : AJson(nullptr);
        return AFuture<AJson>::create([&](AFuture<AJson>::Resolver result) {
            AThread::main()->enqueue([result = std::move(result), handler = std::move(entry->handler),
                                      arguments = std::move(arguments)] {
                try {
                    auto future = handler(arguments);
                    result.onCancel([future] {});   // the call is running while somebody waits for it
                    future.onValue([result](const AJson& value) { result.supplyValue(value); });
                    future.onException([result](const AException& e) {
                        if (auto invocation = dynamic_cast<const AInvocationTargetException*>(&e)) {
                            result.supplyException(invocation->causedBy());
                        } else {
                            result.supplyException(std::make_exception_ptr(AException(e.getMessage())));
                        }
                    });
                } catch (...) {
                    result.supplyException();
                }
            });
        });
    }));
}

}   // namespace aui::remote
