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

#include <gmock/gmock.h>

#include <atomic>
#include <thread>

#include "AUI/Curl/AWebsocket.h"
#include "AUI/Remote/ARemoteAgent.h"
#include "AUI/Remote/Agent/JsonRpc.h"
#include "AUI/UITest.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ATextField.h"

using namespace aui::remote;
using namespace declarative;
using namespace std::chrono_literals;

namespace {
_<ARemoteAgent> agent() {
    ARemoteAgent::Options options;
    options.writeDiscoveryFile = false;
    options.token = "test-token";
    return ARemoteAgent::start(std::move(options));
}
}   // namespace

class AgentTest : public testing::UITest {
protected:
    /// Calls a method and pumps the UI thread until the answer is ready.
    AJson rpc(const AString& method, AJson params = AJson::fromString("{}")) {
        AJson request {
            { "jsonrpc", "2.0" },
            { "id", 1 },
            { "method", method },
            { "params", std::move(params) },
        };
        AString response;
        agent()->dispatcher().handle(AJson::toString(request), [&](AString r) { response = std::move(r); });
        for (int i = 0; i < 200 && response.empty(); ++i) {
            uitest::frame();
        }
        EXPECT_FALSE(response.empty()) << "no response to " << method;
        return AJson::fromString(response.empty() ? AString("{}") : response);
    }

    /// Calls a method that is expected to succeed.
    AJson ok(const AString& method, AJson params = AJson::fromString("{}")) {
        auto response = rpc(method, std::move(params));
        EXPECT_FALSE(response.contains("error")) << AJson::toString(response);
        return response.contains("result") ? response["result"] : AJson(nullptr);
    }

    int errorCode(const AString& method, AJson params = AJson::fromString("{}")) {
        auto response = rpc(method, std::move(params));
        return response.contains("error") ? response["error"]["code"].asInt() : 0;
    }

    _<AWindow> showWindow(_<AView> contents) {
        auto window = _new<AWindow>("Agent test", 400_px, 300_px);
        window->setContents(Centered { std::move(contents) });
        window->show();
        uitest::frame();
        return window;
    }

    /// Finds nodes of the tree by type name.
    static void collect(const AJson& node, const AString& type, AVector<AJson>& out) {
        if (node["type"].asString().contains(type)) {
            out.push_back(node);
        }
        if (auto children = node.containsOpt("children")) {
            for (const auto& child : children->asArray()) {
                collect(child, type, out);
            }
        }
    }
};

TEST_F(AgentTest, HelloListsMethods) {
    auto result = ok("session.hello");
    EXPECT_EQ(result["protocol"].asInt(), 1);
    AVector<AString> methods;
    for (const auto& m : result["methods"].asArray()) {
        methods << m.asString();
    }
    EXPECT_TRUE(methods.contains("session.hello"));
    EXPECT_TRUE(methods.contains("view.tree"));
    EXPECT_TRUE(methods.contains("window.list"));
}

TEST_F(AgentTest, WindowList) {
    auto window = showWindow(Label { "hi" });
    auto windows = ok("window.list");
    ASSERT_TRUE(windows.isArray());
    bool found = false;
    for (const auto& w : windows.asArray()) {
        if (w["title"].asString() == "Agent test") {
            found = true;
            EXPECT_EQ(w["width"].asInt(), window->getSize().x);
            EXPECT_FALSE(w["popup"].asBool());
            EXPECT_TRUE(w["windowId"].asString().startsWith("v"));
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(AgentTest, TreeDescribesViews) {
    auto button = _new<AButton>("Click me");
    auto window = showWindow(Vertical { Label { "Hello" }, button });
    auto tree = ok("view.tree");

    AVector<AJson> buttons;
    collect(tree, "AButton", buttons);
    ASSERT_FALSE(buttons.empty());
    const auto& node = buttons.front();
    EXPECT_EQ(node["role"].asString(), "button");
    EXPECT_TRUE(node["visible"].asBool());
    EXPECT_EQ(node["geometry"]["w"].asInt(), button->getSize().x);
    EXPECT_EQ(node["geometry"]["absX"].asInt(), button->getPositionInWindow().x);

    AVector<AJson> labels;
    collect(tree, "ALabel", labels);
    bool hello = false;
    for (const auto& l : labels) {
        hello |= l.containsOpt("text") && l["text"].asString() == "Hello";
    }
    EXPECT_TRUE(hello);
}

TEST_F(AgentTest, TreeDepthLimitsChildren) {
    auto window = showWindow(Vertical { Label { "Hello" } });
    auto shallow = ok("view.tree", AJson { { "depth", 0 } });
    EXPECT_FALSE(shallow.contains("children"));
    auto deeper = ok("view.tree", AJson { { "depth", 1 } });
    ASSERT_TRUE(deeper.contains("children"));
    for (const auto& child : deeper["children"].asArray()) {
        EXPECT_FALSE(child.contains("children"));
    }
}

TEST_F(AgentTest, TreeCanIncludeInspectorInfo) {
    auto window = showWindow(Label { "Hello" });
    auto without = ok("view.tree");
    EXPECT_FALSE(without.contains("inspector"));
    auto with = ok("view.tree", AJson { { "include", AJson(aui::impl::JsonArray { AJson("inspector") }) } });
    ASSERT_TRUE(with.contains("inspector"));
    EXPECT_TRUE(with["inspector"].contains("Size"));
}

TEST_F(AgentTest, FindByTextTypeAndInteractivity) {
    auto window = showWindow(Vertical { Label { "Hello" }, _new<AButton>("Click me"), _new<ATextField>() });
    auto byText = ok("view.find", AJson { { "text", "click" } });   // case-insensitive
    ASSERT_FALSE(byText.asArray().empty());
    bool hasButton = false;
    for (const auto& n : byText.asArray()) {
        hasButton |= n["type"].asString().contains("AButton");
    }
    EXPECT_TRUE(hasButton);

    auto byType = ok("view.find", AJson { { "type", "textfield" } });
    EXPECT_FALSE(byType.asArray().empty());

    auto interactive = ok("view.find", AJson { { "interactive", true } });
    for (const auto& n : interactive.asArray()) {
        EXPECT_TRUE(n.contains("role"));
    }
    EXPECT_GE(interactive.asArray().size(), 2u);   // the button and the text field
}

TEST_F(AgentTest, InteractiveListsRolesLabelsAndActions) {
    auto window = showWindow(Vertical { Label { "Static" }, _new<AButton>("Click me"), _new<ATextField>() });
    auto elements = ok("view.interactive");
    ASSERT_TRUE(elements.isArray());

    const AJson* button = nullptr;
    const AJson* textbox = nullptr;
    for (const auto& e : elements.asArray()) {
        if (e["role"].asString() == "button") button = &e;
        if (e["role"].asString() == "textbox") textbox = &e;
    }
    ASSERT_TRUE(button);
    ASSERT_TRUE(textbox);
    EXPECT_EQ((*button)["label"].asString(), "Click me");
    EXPECT_TRUE((*button)["rect"]["w"].asInt() > 0);
    bool canType = false;
    for (const auto& a : (*textbox)["actions"].asArray()) {
        canType |= a.asString() == "type";
    }
    EXPECT_TRUE(canType);
}

TEST_F(AgentTest, InteractiveSkipsDisabledAndHidden) {
    auto disabled = _new<AButton>("Disabled");
    disabled->setEnabled(false);
    auto hidden = _new<AButton>("Hidden");
    hidden->setVisibility(Visibility::GONE);
    auto window = showWindow(Vertical { disabled, hidden, _new<AButton>("Fine") });
    auto elements = ok("view.interactive");
    AVector<AString> labels;
    for (const auto& e : elements.asArray()) {
        labels << e["label"].asString();
    }
    EXPECT_TRUE(labels.contains("Fine"));
    EXPECT_FALSE(labels.contains("Disabled"));
    EXPECT_FALSE(labels.contains("Hidden"));
}

TEST_F(AgentTest, HitTestReturnsPathFromTheWindow) {
    auto button = _new<AButton>("Click me");
    auto window = showWindow(button);
    auto center = button->getCenterPointInWindow();
    auto result = ok("view.hitTest", AJson { { "x", center.x }, { "y", center.y } });
    auto path = result["path"].asArray();
    ASSERT_GE(path.size(), 2u);
    EXPECT_TRUE(path.front()["type"].asString().contains("AWindow"));
    bool throughButton = false;
    for (const auto& n : path) {
        throughButton |= n["type"].asString().contains("AButton");
    }
    EXPECT_TRUE(throughButton);
}

TEST_F(AgentTest, HitTestOutsideOfViewsIsEmpty) {
    auto window = showWindow(_new<AButton>("Click me"));
    auto result = ok("view.hitTest", AJson { { "x", -50 }, { "y", -50 } });
    EXPECT_TRUE(result["path"].asArray().empty());
}

TEST_F(AgentTest, PropertiesOfAView) {
    auto field = _new<ATextField>();
    field->setText("typed");
    auto window = showWindow(field);
    auto found = ok("view.find", AJson { { "type", "textfield" } });
    auto ref = found.asArray().front()["ref"];
    auto properties = ok("view.properties", AJson { { "ref", ref } });
    EXPECT_EQ(properties["inspector"]["text"].asString(), "typed");
    EXPECT_EQ(properties["role"].asString(), "textbox");
}

TEST_F(AgentTest, RefsOfRemovedViewsAreStale) {
    auto window = showWindow(Vertical { _new<AButton>("Click me") });
    auto found = ok("view.find", AJson { { "type", "AButton" } });
    auto ref = found.asArray().front()["ref"];
    EXPECT_EQ(errorCode("view.properties", AJson { { "ref", ref } }), 0);

    window->setContents(Centered { Label { "gone" } });
    uitest::frame();
    EXPECT_EQ(errorCode("view.properties", AJson { { "ref", ref } }), rpc_code::STALE_REF);
}

TEST_F(AgentTest, BadRefsAndMissingParams) {
    EXPECT_EQ(errorCode("view.properties", AJson { { "ref", "garbage" } }), rpc_code::STALE_REF);
    EXPECT_EQ(errorCode("view.properties"), rpc_code::INVALID_PARAMS);
}

TEST_F(AgentTest, WindowIdMustBeAWindow) {
    auto button = _new<AButton>("Click me");
    auto window = showWindow(button);
    auto found = ok("view.find", AJson { { "type", "AButton" } });
    auto buttonRef = found.asArray().front()["ref"];
    EXPECT_EQ(errorCode("view.tree", AJson { { "windowId", buttonRef } }), rpc_code::NO_SUCH_WINDOW);
}

TEST_F(AgentTest, ListedWindowIdWorks) {
    auto window = showWindow(Label { "hi" });
    auto windows = ok("window.list");
    for (const auto& w : windows.asArray()) {
        if (w["title"].asString() == "Agent test") {
            auto tree = ok("view.tree", AJson { { "windowId", w["windowId"] }, { "depth", 0 } });
            EXPECT_TRUE(tree["type"].asString().contains("AWindow"));
        }
    }
}

namespace {
/// Connects to the agent over a real WebSocket on a background thread while the test pumps the UI thread.
struct WebsocketSession {
    std::atomic<bool> finished = false;
    std::atomic<bool> connected = false;
    AString reply;
    std::thread thread;

    WebsocketSession(const AString& url, AString request) {
        thread = std::thread([this, url, request = std::move(request)] {
            try {
                auto ws = _new<AWebsocket>(url);
                AObject::connect(ws->connected, ws, [&, ws] {
                    connected = true;
                    ws->writeText(request);
                });
                AObject::connect(ws->received, ws, [&, ws](AByteBufferView payload) {
                    reply = AString::fromUtf8(payload);
                    ws->close();
                });
                ws->run();
            } catch (const AException&) {
            }
            finished = true;
        });
    }
    ~WebsocketSession() { thread.join(); }

    void waitPumping() {
        for (int i = 0; i < 12000 && !finished; ++i) {   // up to a minute: loopback connect() may be slow behind a VPN
            uitest::frame();
            std::this_thread::sleep_for(5ms);
        }
    }
};
}   // namespace

TEST_F(AgentTest, FullStackOverWebsocket) {
    auto window = showWindow(_new<AButton>("Click me"));
    const auto url = "ws://127.0.0.1:" + AString::number(agent()->port()) + "/?token=test-token";
    WebsocketSession session(url, R"({"jsonrpc":"2.0","id":1,"method":"view.interactive"})");
    session.waitPumping();
    ASSERT_TRUE(session.finished);
    ASSERT_FALSE(session.reply.empty());
    auto response = AJson::fromString(session.reply);
    bool found = false;
    for (const auto& e : response["result"].asArray()) {
        found |= e["label"].asString() == "Click me";
    }
    EXPECT_TRUE(found) << session.reply;
}

TEST_F(AgentTest, WrongTokenIsRejected) {
    const auto url = "ws://127.0.0.1:" + AString::number(agent()->port()) + "/?token=wrong";
    WebsocketSession session(url, R"({"jsonrpc":"2.0","id":1,"method":"session.hello"})");
    session.waitPumping();
    EXPECT_TRUE(session.finished);
    EXPECT_FALSE(session.connected);
    EXPECT_TRUE(session.reply.empty());
}

TEST_F(AgentTest, MissingTokenIsRejected) {
    const auto url = "ws://127.0.0.1:" + AString::number(agent()->port()) + "/";
    WebsocketSession session(url, R"({"jsonrpc":"2.0","id":1,"method":"session.hello"})");
    session.waitPumping();
    EXPECT_FALSE(session.connected);
}
