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

#include <gtest/gtest.h>
#include "AUI/Remote/Agent/JsonRpc.h"

using namespace aui::remote;

namespace {
/// Sends the message and returns the response, or "<none>" if the dispatcher didn't answer (yet).
AString call(const JsonRpcDispatcher& dispatcher, const AString& message) {
    AString result = "<none>";
    dispatcher.handle(message, [&](AString response) { result = std::move(response); });
    return result;
}

AJson callJson(const JsonRpcDispatcher& dispatcher, const AString& message) {
    return AJson::fromString(call(dispatcher, message));
}

std::unique_ptr<JsonRpcDispatcher> makeDispatcher() {
    auto holder = std::make_unique<JsonRpcDispatcher>();
    auto& d = *holder;
    d.registerMethod("echo", JsonRpcDispatcher::SyncHandler([](const AJson& params) { return params; }));
    d.registerMethod("add", JsonRpcDispatcher::SyncHandler([](const AJson& params) {
        return AJson(params["a"].asInt() + params["b"].asInt());
    }));
    d.registerMethod("fail", JsonRpcDispatcher::SyncHandler([](const AJson&) -> AJson {
        throw RpcException(rpc_code::STALE_REF, "view is gone", "run view.interactive again");
    }));
    d.registerMethod("crash", JsonRpcDispatcher::SyncHandler([](const AJson&) -> AJson { throw std::runtime_error("boom"); }));
    return holder;
}
}   // namespace

TEST(JsonRpc, Success) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    auto r = callJson(d, R"({"jsonrpc":"2.0","method":"add","params":{"a":2,"b":3},"id":7})");
    EXPECT_EQ(r["jsonrpc"].asString(), "2.0");
    EXPECT_EQ(r["id"].asInt(), 7);
    EXPECT_EQ(r["result"].asInt(), 5);
    EXPECT_FALSE(r.contains("error"));
}

TEST(JsonRpc, StringAndNullIds) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    EXPECT_EQ(callJson(d, R"({"jsonrpc":"2.0","method":"echo","id":"abc"})")["id"].asString(), "abc");
    EXPECT_TRUE(callJson(d, R"({"jsonrpc":"2.0","method":"echo","id":null})")["id"].isNull());
}

TEST(JsonRpc, MissingParamsAreAnEmptyObject) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    auto r = callJson(d, R"({"jsonrpc":"2.0","method":"echo","id":1})");
    EXPECT_TRUE(r["result"].isObject());
}

TEST(JsonRpc, NotificationsAreNotAnswered) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    int calls = 0;
    d.registerMethod("count", JsonRpcDispatcher::SyncHandler([&](const AJson&) { ++calls; return AJson(nullptr); }));
    EXPECT_EQ(call(d, R"({"jsonrpc":"2.0","method":"count"})"), "<none>");
    EXPECT_EQ(calls, 1);
}

TEST(JsonRpc, ParseError) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    auto r = callJson(d, "{not json");
    EXPECT_EQ(r["error"]["code"].asInt(), rpc_code::PARSE_ERROR);
    EXPECT_TRUE(r["id"].isNull());
}

TEST(JsonRpc, InvalidRequests) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    {
        // not an object (the parser doesn't accept bare scalars, so check it inside of a batch)
        auto r = callJson(d, "[42]");
        ASSERT_TRUE(r.isArray());
        EXPECT_EQ(r[0]["error"]["code"].asInt(), rpc_code::INVALID_REQUEST);
    }
    EXPECT_EQ(callJson(d, R"({"method":"echo","id":1})")["error"]["code"].asInt(), rpc_code::INVALID_REQUEST);
    EXPECT_EQ(callJson(d, R"({"jsonrpc":"1.0","method":"echo","id":1})")["error"]["code"].asInt(), rpc_code::INVALID_REQUEST);
    EXPECT_EQ(callJson(d, R"({"jsonrpc":"2.0","method":5,"id":1})")["error"]["code"].asInt(), rpc_code::INVALID_REQUEST);
    EXPECT_EQ(callJson(d, R"({"jsonrpc":"2.0","method":"echo","id":[1]})")["error"]["code"].asInt(), rpc_code::INVALID_REQUEST);
    EXPECT_EQ(callJson(d, R"({"jsonrpc":"2.0","method":"echo","params":5,"id":1})")["error"]["code"].asInt(), rpc_code::INVALID_PARAMS);
}

TEST(JsonRpc, MethodNotFound) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    auto r = callJson(d, R"({"jsonrpc":"2.0","method":"nope","id":1})");
    EXPECT_EQ(r["error"]["code"].asInt(), rpc_code::METHOD_NOT_FOUND);
    EXPECT_EQ(r["id"].asInt(), 1);
}

TEST(JsonRpc, HandlerErrorsKeepCodeAndHint) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    auto r = callJson(d, R"({"jsonrpc":"2.0","method":"fail","id":1})");
    EXPECT_EQ(r["error"]["code"].asInt(), rpc_code::STALE_REF);
    EXPECT_EQ(r["error"]["message"].asString(), "view is gone");
    EXPECT_EQ(r["error"]["data"]["hint"].asString(), "run view.interactive again");
}

TEST(JsonRpc, TypeMismatchIsInvalidParams) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    auto r = callJson(d, R"({"jsonrpc":"2.0","method":"add","params":{"a":"x","b":3},"id":1})");
    EXPECT_EQ(r["error"]["code"].asInt(), rpc_code::INVALID_PARAMS);
}

TEST(JsonRpc, OtherExceptionsAreInternalErrors) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    auto r = callJson(d, R"({"jsonrpc":"2.0","method":"crash","id":1})");
    EXPECT_EQ(r["error"]["code"].asInt(), rpc_code::INTERNAL_ERROR);
    EXPECT_EQ(r["error"]["message"].asString(), "boom");
}

TEST(JsonRpc, AsyncHandlerAnswersLater) {
    JsonRpcDispatcher d;
    AFuture<AJson> pending;
    d.registerMethod("later", JsonRpcDispatcher::Handler([&](const AJson&) { return pending; }));
    AString result = "<none>";
    d.handle(R"({"jsonrpc":"2.0","method":"later","id":1})", [&](AString r) { result = std::move(r); });
    EXPECT_EQ(result, "<none>");
    pending.supplyValue(AJson("done"));
    EXPECT_EQ(AJson::fromString(result)["result"].asString(), "done");
}

TEST(JsonRpc, AsyncHandlerErrorKeepsCode) {
    JsonRpcDispatcher d;
    AFuture<AJson> pending;
    d.registerMethod("later", JsonRpcDispatcher::Handler([&](const AJson&) { return pending; }));
    AString result = "<none>";
    d.handle(R"({"jsonrpc":"2.0","method":"later","id":1})", [&](AString r) { result = std::move(r); });
    try {
        throw RpcException(rpc_code::TIMEOUT, "too slow");
    } catch (...) {
        pending.supplyException();
    }
    auto r = AJson::fromString(result);
    EXPECT_EQ(r["error"]["code"].asInt(), rpc_code::TIMEOUT);
    EXPECT_EQ(r["error"]["message"].asString(), "too slow");
}

TEST(JsonRpc, Batch) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    auto r = callJson(d, R"([
        {"jsonrpc":"2.0","method":"add","params":{"a":1,"b":2},"id":1},
        {"jsonrpc":"2.0","method":"echo"},
        {"jsonrpc":"2.0","method":"nope","id":2}
    ])");
    ASSERT_TRUE(r.isArray());
    ASSERT_EQ(r.asArray().size(), 2u);   // the notification has no response
    EXPECT_EQ(r[0]["result"].asInt(), 3);
    EXPECT_EQ(r[1]["error"]["code"].asInt(), rpc_code::METHOD_NOT_FOUND);
}

TEST(JsonRpc, EmptyBatchIsInvalid) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    EXPECT_EQ(callJson(d, "[]")["error"]["code"].asInt(), rpc_code::INVALID_REQUEST);
}

TEST(JsonRpc, BatchOfNotificationsIsNotAnswered) {
    auto dispatcher = makeDispatcher();
    auto& d = *dispatcher;
    EXPECT_EQ(call(d, R"([{"jsonrpc":"2.0","method":"echo"},{"jsonrpc":"2.0","method":"echo"}])"), "<none>");
}
