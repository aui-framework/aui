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
#include <fstream>
#include <sstream>
#include <thread>

#include "AUI/Crypt/ATlsSession.h"
#include "AUI/Curl/AWebsocket.h"
#include "AUI/Network/AInet4Address.h"
#include "AUI/Network/ATcpSocket.h"
#include "AUI/Remote/Agent/Pairing.h"
#include "AUI/Remote/ARemoteDebug.h"
#include "AUI/Remote/Agent/JsonRpc.h"
#include "AUI/Remote/Agent/RemoteOverlay.h"
#include "AUI/Remote/Agent/TreeDiff.h"
#include <condition_variable>
#include "AUI/UITest.h"
#include "AUI/Image/AImage.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/A2FingerTransformArea.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ALabel.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/View/ATextField.h"

using namespace aui::remote;
using namespace declarative;
using namespace std::chrono_literals;

namespace {
constexpr auto KEY = "test-key-0123456789abcdef";

_<ARemoteDebug> agent() {
    ARemoteDebug::Options options;
    options.writeDiscoveryFile = false;
    options.pairing = ARemoteDebug::PairingPolicy::SESSION;   // keep ~/.aui untouched
    options.key = KEY;
    return ARemoteDebug::start(std::move(options));
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
        for (int i = 0; i < 2000 && response.empty(); ++i) {
            uitest::frame();
            if (response.empty()) {
                std::this_thread::sleep_for(2ms);   // methods like wait.* poll in real time
            }
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
    EXPECT_EQ(result["defaultPointer"].asString(), "mouse");   // tests run on desktop platforms
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
        canType |= a.asString() == "typeable";
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

TEST_F(AgentTest, ScreenshotSavesPngAndCrops) {
    auto window = showWindow(_new<AButton>("Shot"));
    auto full = ok("screenshot");
    const auto fullPath = full["path"].asString();
    EXPECT_TRUE(APath(fullPath).isRegularFileExists());
    EXPECT_TRUE(fullPath.endsWith(".png"));
    EXPECT_GT(full["width"].asInt(), 0);

    auto ref = ok("view.find", AJson { { "type", "AButton" } }).asArray().front()["ref"].asString();
    auto cropped = ok("screenshot", AJson { { "ref", ref } });
    EXPECT_TRUE(APath(cropped["path"].asString()).isRegularFileExists());
    EXPECT_LT(cropped["width"].asInt(), full["width"].asInt());
    EXPECT_LT(cropped["height"].asInt(), full["height"].asInt());
    EXPECT_NE(cropped["path"].asString(), fullPath);
}

TEST_F(AgentTest, AnnotatedScreenshotMarksInteractiveViews) {
    int clicks = 0;
    auto second = _new<AButton>("Second");
    AObject::connect(second->clicked, second, [&] { ++clicks; });
    auto window = showWindow(Vertical {
      _new<AButton>("First"),
      second,
      _new<ATextField>(),
      Label { "text" },
    });
    const auto plainBefore = ok("screenshot");
    const auto interactive = ok("view.interactive").asArray();
    const auto annotated = ok("screenshot", AJson { { "annotate", true } });
    const auto& marks = annotated["marks"].asArray();
    // every mark is one of the interactive views; the labels inside the buttons are not marked separately
    AVector<AString> texts;
    for (std::size_t i = 0; i < marks.size(); ++i) {
        EXPECT_EQ(marks[i]["label"].asInt(), int(i) + 1);
        const bool listed = std::any_of(interactive.begin(), interactive.end(), [&](const AJson& e) {
            return e["role"].asString() == marks[i]["role"].asString() &&
                   e["rect"]["absX"].asInt() == marks[i]["rect"]["x"].asInt() &&
                   e["rect"]["absY"].asInt() == marks[i]["rect"]["y"].asInt();
        });
        EXPECT_TRUE(listed) << AJson::toString(marks[i]);
        texts << marks[i]["text"].asString();
    }
    EXPECT_EQ(std::count(texts.begin(), texts.end(), "First"), 1);
    EXPECT_EQ(std::count(texts.begin(), texts.end(), "Second"), 1);
    EXPECT_TRUE(std::any_of(marks.begin(), marks.end(), [](const AJson& m) { return m["role"].asString() == "textbox"; }));

    // the refs of the marks work right after the call
    for (const auto& mark : marks) {
        if (mark["text"].asString() == "Second") {
            ok("input.pointer", AJson { { "ref", mark["ref"] } });
        }
    }
    EXPECT_EQ(clicks, 1);

    // the marks change the picture only around the marked views, and a plain screenshot stays plain
    auto plain = AImage::fromFile(APath(plainBefore["path"].asString()));
    auto withMarks = AImage::fromFile(APath(annotated["path"].asString()));
    auto plainAfter = AImage::fromFile(APath(ok("screenshot")["path"].asString()));
    ASSERT_EQ(plain->size(), withMarks->size());
    const float scale = annotated["scale"].asNumber();
    const int reach = int(40 * scale);   // the box line and the number plate around the box
    int changed = 0;
    for (unsigned y = 0; y < plain->height(); ++y) {
        for (unsigned x = 0; x < plain->width(); ++x) {
            EXPECT_EQ(plain->get({ x, y }), plainAfter->get({ x, y })) << x << "," << y;
            if (plain->get({ x, y }) == withMarks->get({ x, y })) {
                continue;
            }
            ++changed;
            const glm::vec2 p = glm::vec2(x, y) / scale;
            const bool nearMark = std::any_of(marks.begin(), marks.end(), [&](const AJson& m) {
                const auto& r = m["rect"];
                return p.x >= r["x"].asInt() - reach && p.y >= r["y"].asInt() - reach &&
                       p.x <= r["x"].asInt() + r["w"].asInt() + reach && p.y <= r["y"].asInt() + r["h"].asInt() + reach;
            });
            ASSERT_TRUE(nearMark) << "changed pixel far from the marks: " << x << "," << y;
        }
    }
    EXPECT_GT(changed, 0);
}

TEST_F(AgentTest, ClickInvokesButton) {
    int clicks = 0;
    auto button = _new<AButton>("Click me");
    AObject::connect(button->clicked, button, [&] { ++clicks; });
    auto window = showWindow(button);
    auto ref = ok("view.find", AJson { { "type", "AButton" } }).asArray().front()["ref"].asString();
    ok("input.pointer", AJson { { "ref", ref } });
    EXPECT_EQ(clicks, 1);
    ok("input.pointer", AJson { { "ref", ref }, { "action", "click" } });
    EXPECT_EQ(clicks, 2);
}

TEST_F(AgentTest, ClickByCoordinates) {
    int clicks = 0;
    auto button = _new<AButton>("Click me");
    AObject::connect(button->clicked, button, [&] { ++clicks; });
    auto window = showWindow(button);
    const auto center = button->getCenterPointInWindow();
    ok("input.pointer", AJson { { "x", center.x }, { "y", center.y } });
    EXPECT_EQ(clicks, 1);
    ok("input.pointer", AJson { { "x", 1 }, { "y", 1 } });
    EXPECT_EQ(clicks, 1);
}

TEST_F(AgentTest, WaitIdleDoesNotWaitForTheBanner) {
    auto window = showWindow(_new<AButton>("b"));
    ok("input.pointer", AJson { { "x", 200 }, { "y", 150 } });
    EXPECT_TRUE(RemoteOverlay::isShown(*window));
    // the banner lives for 3 seconds; valgrind is slow enough to take seconds to get the UI idle
    EXPECT_LT(ok("wait.idle", AJson { { "quietMs", 30 } })["waitedMs"].asInt(), 3000);
}

TEST_F(AgentTest, ResizeHasAnUpperLimit) {
    auto window = showWindow(_new<AButton>("b"));
    // the stub window manager packs the window, see WindowResize; the size that was asked for can't be observed
    auto result = ok("window.resize", AJson { { "width", 1000000 }, { "height", 100 } });
    EXPECT_LE(result["width"].asInt(), 16384);
    EXPECT_EQ(result["requestedWidth"].asInt(), 1000000);
    EXPECT_EQ(result["maxWidth"].asInt(), 16384);
}

TEST_F(AgentTest, ClickHoldsTheButtonBeforeRelease) {
    auto button = _new<AButton>("Click me");
    auto window = showWindow(button);
    auto ref = ok("view.find", AJson { { "type", "AButton" } }).asArray().front()["ref"].asString();
    const auto start = std::chrono::steady_clock::now();
    ok("input.pointer", AJson { { "ref", ref } });
    EXPECT_GE(std::chrono::steady_clock::now() - start, 50ms);
    EXPECT_FALSE(button->isPressed());
}

TEST_F(AgentTest, MoveHoversWithoutClick) {
    int clicks = 0;
    auto button = _new<AButton>("Hover me");
    AObject::connect(button->clicked, button, [&] { ++clicks; });
    auto window = showWindow(button);
    uitest::frame();
    EXPECT_FALSE(button->isMouseHover());
    auto ref = ok("view.find", AJson { { "type", "AButton" } }).asArray().front()["ref"].asString();
    ok("input.pointer", AJson { { "ref", ref }, { "action", "move" } });
    EXPECT_TRUE(button->isMouseHover());
    EXPECT_EQ(clicks, 0);
    ok("input.pointer", AJson { { "x", 1 }, { "y", 1 }, { "action", "move" } });
    EXPECT_FALSE(button->isMouseHover());
}

TEST_F(AgentTest, FingerTapClicks) {
    int clicks = 0;
    auto button = _new<AButton>("Tap me");
    AObject::connect(button->clicked, button, [&] { ++clicks; });
    auto window = showWindow(button);
    uitest::frame();
    auto ref = ok("view.find", AJson { { "type", "AButton" } }).asArray().front()["ref"].asString();
    ok("input.pointer", AJson { { "ref", ref }, { "pointer", "finger" } });
    EXPECT_EQ(clicks, 1);
    EXPECT_FALSE(AInput::isKeyDown(AInput::LBUTTON));
    EXPECT_NE(errorCode("input.pointer", AJson { { "ref", ref }, { "pointer", "pen" } }), 0);
}

TEST_F(AgentTest, OverlayIsOnAndScreenshotsDontLeaveItHidden) {
    auto window = showWindow(_new<AButton>("Overlay"));
    ok("input.pointer", AJson { { "x", 5 }, { "y", 5 }, { "action", "move" } });   // remote input turns it on
    EXPECT_TRUE(RemoteOverlay::isEnabled(*window));
    ok("screenshot");
    EXPECT_TRUE(RemoteOverlay::isEnabled(*window));
    ok("screenshot");
    EXPECT_TRUE(RemoteOverlay::isEnabled(*window));
}

TEST_F(AgentTest, TypeIntoTextField) {
    auto field = _new<ATextField>();
    auto window = showWindow(field);
    auto ref = ok("view.find", AJson { { "type", "ATextField" } }).asArray().front()["ref"].asString();
    ok("input.type", AJson { { "ref", ref }, { "text", "hello" } });
    EXPECT_EQ(field->text(), "hello");
    ok("input.key", AJson { { "key", "BACKSPACE" } });
    EXPECT_EQ(field->text(), "hell");
#if AUI_PLATFORM_APPLE
    ok("input.key", AJson { { "key", "A" }, { "modifiers", AJson::fromString(R"(["cmd"])") } });
#else
    ok("input.key", AJson { { "key", "A" }, { "modifiers", AJson::fromString(R"(["ctrl"])") } });
#endif
    ok("input.type", AJson { { "text", "x" } });
    EXPECT_EQ(field->text(), "x");
}

TEST_F(AgentTest, KeyboardGoesToTheWindowOfTheRef) {
    auto mainField = _new<ATextField>();
    auto window = showWindow(mainField);
    auto dialogField = _new<ATextField>();
    dialogField->setText("dialog");
    auto dialog = showWindow(dialogField);

    const auto dialogId = ok("window.list").asArray().back()["windowId"];
    const auto found = ok("view.find", AJson { { "text", "dialog" }, { "windowId", dialogId } }).asArray();
    ASSERT_EQ(found.size(), 1u);
    const auto ref = found.front()["ref"].asString();
    ok("input.type", AJson { { "ref", ref }, { "text", "!" } });
    EXPECT_EQ(dialogField->text(), "dialog!");
    EXPECT_EQ(mainField->text(), "");
    ok("input.key", AJson { { "ref", ref }, { "key", "BACKSPACE" } });
    EXPECT_EQ(dialogField->text(), "dialog");
}

TEST_F(AgentTest, Drag) {
    auto view = _new<AView>();
    int pressed = 0, released = 0;
    auto window = showWindow(view AUI_OVERRIDE_STYLE { ass::FixedSize { 200_px, 100_px } });
    AObject::connect(view->pressed, AObject::GENERIC_OBSERVER, [&] { ++pressed; });
    AObject::connect(view->released, AObject::GENERIC_OBSERVER, [&] { ++released; });
    const auto at = glm::ivec2(view->getCenterPointInWindow());

    ok("input.pointer", AJson { { "action", "drag" }, { "x", at.x - 20 }, { "y", at.y }, { "toX", at.x + 20 }, { "toY", at.y } });
    EXPECT_EQ(pressed, 1);
    EXPECT_EQ(released, 1);

    const auto ref = ok("view.find", AJson { { "type", "AView" } }).asArray().front()["ref"].asString();
    ok("input.pointer", AJson { { "action", "drag" }, { "x", at.x - 20 }, { "y", at.y }, { "toRef", ref } });
    EXPECT_EQ(pressed, 2);
    EXPECT_EQ(released, 2);
}

TEST_F(AgentTest, PlainLabelsAreNotInteractive) {
    auto window = showWindow(Vertical {
      Label { "text" },
      _new<AButton>("b"),
    });
    const auto interactive = ok("view.interactive");
    for (const auto& element : interactive.asArray()) {
        EXPECT_NE(element["label"].asString(), "text") << AJson::toString(element);
    }
}

TEST_F(AgentTest, InputErrors) {
    auto window = showWindow(_new<AButton>("b"));
    EXPECT_EQ(errorCode("input.key", AJson { { "key", "NO_SUCH_KEY" } }), rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("input.pointer", AJson { { "action", "teleport" }, { "x", 1 }, { "y", 1 } }),
              rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("input.pointer", AJson { { "ref", "v0:99999" } }), rpc_code::STALE_REF);
    EXPECT_EQ(errorCode("input.type", AJson::fromString("{}")), rpc_code::INVALID_PARAMS);
}

TEST_F(AgentTest, WaitIdleResolves) {
    auto window = showWindow(Label { "hi" });
    auto result = ok("wait.idle", AJson { { "quietMs", 30 } });
    EXPECT_TRUE(result["idle"].asBool());
}

TEST_F(AgentTest, WaitViewAppearsAndDisappears) {
    auto label = _new<ALabel>("later");
    _<AViewContainer> container = Vertical {};
    auto window = showWindow(container);
    EXPECT_EQ(errorCode("wait.view", AJson { { "text", "later" }, { "timeoutMs", 50 } }), rpc_code::TIMEOUT);

    AThread::main()->enqueue([&] { container->addView(label); });
    auto result = ok("wait.view", AJson { { "text", "later" } });
    EXPECT_EQ(result["matches"].asArray().size(), 1u);

    AThread::main()->enqueue([&] { container->removeView(label); });
    ok("wait.view", AJson { { "text", "later" }, { "state", "gone" } });
}

TEST_F(AgentTest, RegisteredFunctionsAreCallable) {
    agent()->registerFunction("echo", "returns its arguments", [](const AJson& args) {
        return AFuture<AJson>::fromValue(AJson { { "echo", args["value"] } });
    });
    agent()->registerFunction("fail", "always fails", [](const AJson&) {
        return AFuture<AJson>::fromException(std::make_exception_ptr(AException("boom")));
    });
    auto list = ok("fn.list");
    bool found = false;
    for (const auto& f : list.asArray()) {
        found |= f["name"].asString() == "echo";
    }
    EXPECT_TRUE(found);
    EXPECT_EQ(ok("fn.call", AJson { { "name", "echo" }, { "arguments", AJson { { "value", 42 } } } })["echo"].asInt(), 42);
    EXPECT_EQ(errorCode("fn.call", AJson { { "name", "fail" } }), rpc_code::INTERNAL_ERROR);
    EXPECT_EQ(errorCode("fn.call", AJson { { "name", "nope" } }), rpc_code::UNKNOWN_FUNCTION);
}

TEST_F(AgentTest, LogIsAvailable) {
    ALogger::info("AgentTestTag") << "hello from the test";
    auto result = ok("diag.log", AJson { { "limit", 50 } });
    bool found = false;
    for (const auto& e : result["entries"].asArray()) {
        found |= e["message"].asString().contains("hello from the test") && e["tag"].asString() == "AgentTestTag";
    }
    EXPECT_TRUE(found) << AJson::toString(result);
    const auto next = result["next"].asInt();
    EXPECT_EQ(ok("diag.log", AJson { { "since", next } })["entries"].asArray().size(), 0u);
    EXPECT_EQ(errorCode("diag.log", AJson { { "level", "loud" } }), rpc_code::INVALID_PARAMS);
}

TEST_F(AgentTest, WindowResize) {
    // the stub window manager packs windows to their minimum size on every frame, so a resize below the minimum is what
    // can be observed here; growing a window is checked against a real window manager
    auto window = showWindow(Label { "a label wide enough to have a minimum width" });
    auto result = ok("window.resize", AJson { { "width", 1 }, { "height", 1 } });
    EXPECT_EQ(result["width"].asInt(), window->getSize().x);
    EXPECT_EQ(result["height"].asInt(), window->getSize().y);
    EXPECT_EQ(result["requestedWidth"].asInt(), 1);
    EXPECT_EQ(result["requestedHeight"].asInt(), 1);
    EXPECT_EQ(result["minWidth"].asInt(), window->getMinimumSize().x);
    EXPECT_GE(result["width"].asInt(), result["minWidth"].asInt());

    EXPECT_EQ(errorCode("window.resize", AJson { { "width", 0 }, { "height", 100 } }), rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("window.resize", AJson { { "width", 100 } }), rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("window.resize", AJson { { "width", 100 }, { "height", 100 }, { "windowId", "v0:99999" } }),
              rpc_code::STALE_REF);
}

TEST_F(AgentTest, LongPress) {
    auto view = _new<ALabel>("hold me") AUI_OVERRIDE_STYLE { FixedSize { 100_px, 40_px } };
    int longPressed = 0, clicked = 0;
    AObject::connect(view->clickedRightOrLongPressed, view, [&] { ++longPressed; });
    AObject::connect(view->clicked, view, [&] { ++clicked; });
    auto window = showWindow(view);
    const auto ref = ok("view.find", AJson { { "text", "hold me" } })[0]["ref"].asString();

    // even with a mouse as the default pointer, a long press is a touch gesture
    auto result = ok("input.pointer", AJson { { "ref", ref }, { "action", "longPress" }, { "durationMs", 50 } });
    EXPECT_TRUE(result["handled"].asBool());
    EXPECT_EQ(longPressed, 1);
    EXPECT_EQ(clicked, 0) << "a handled long press is not followed by a click";

    // the gesture alone, as the GUI sends it while it holds the finger itself
    EXPECT_TRUE(ok("input.gesture", AJson { { "gesture", "longPress" }, { "ref", ref } })["handled"].asBool());
    EXPECT_EQ(longPressed, 2);
    EXPECT_EQ(errorCode("input.gesture", AJson { { "gesture", "tap" }, { "ref", ref } }), rpc_code::INVALID_PARAMS);
}

TEST_F(AgentTest, FlingScrollsFurtherThanTheFinger) {
    _<AViewContainer> list = Vertical {};
    for (int i = 0; i < 200; ++i) {
        list->addView(Label { "item {}"_format(i) });
    }
    auto area = AScrollArea::Builder().withContents(list).build();
    auto window = showWindow(area AUI_OVERRIDE_STYLE { FixedSize { 200_px, 300_px } });
    const auto center = area->getCenterPointInWindow();

    auto result = ok("input.pointer", AJson { { "x", center.x }, { "y", center.y + 100 }, { "action", "fling" },
                                              { "dy", -200 }, { "velocity", 3000 } });
    EXPECT_NEAR(result["velocity"].asInt(), 3000, 100);
    const auto afterFinger = area->verticalScrollbar()->getCurrentScroll();
    EXPECT_GE(afterFinger, 180);   // the finger itself moved the contents (minus the touch slop)
    for (int i = 0; i < 30; ++i) {
        uitest::frame();   // the kinetic scroll goes on frame by frame
    }
    EXPECT_GT(area->verticalScrollbar()->getCurrentScroll(), afterFinger + 50);
}

TEST_F(AgentTest, PinchAndRotateReachA2FingerTransformArea) {
    auto area = _new<A2FingerTransformArea>() AUI_OVERRIDE_STYLE { FixedSize { 300_px, 300_px } };
    A2DTransform total;
    AObject::connect(area->transformed, area, [&](const A2DTransform& t) { total.applyDelta(t); });
    auto window = showWindow(area);
    const auto center = area->getCenterPointInWindow();

    auto pinch = ok("input.touch", AJson { { "gesture", "pinch" }, { "x", center.x }, { "y", center.y }, { "from", 50 },
                                           { "to", 150 }, { "durationMs", 100 } });
    EXPECT_EQ(pinch["fingers"].asInt(), 2);
    EXPECT_NEAR(total.scale, 3.f, 0.1f);

    total = {};
    ok("input.touch", AJson { { "gesture", "rotate" }, { "x", center.x }, { "y", center.y }, { "degrees", 90 },
                              { "durationMs", 100 } });
    EXPECT_NEAR(std::abs(total.rotation.degrees()), 90.f, 2.f);
    EXPECT_NEAR(total.scale, 1.f, 0.05f);
}

TEST_F(AgentTest, TouchTracks) {
    auto area = _new<A2FingerTransformArea>() AUI_OVERRIDE_STYLE { FixedSize { 300_px, 300_px } };
    A2DTransform total;
    AObject::connect(area->transformed, area, [&](const A2DTransform& t) { total.applyDelta(t); });
    auto window = showWindow(area);
    const auto c = area->getCenterPointInWindow();

    // two fingers moving together pan
    auto result = ok("input.touch", AJson::fromString(R"({"fingers":[
        {"points":[{"x":)" + AString::number(c.x - 20) + R"(,"y":)" + AString::number(c.y) + R"(,"t":0},
                   {"x":)" + AString::number(c.x - 20) + R"(,"y":)" + AString::number(c.y + 40) + R"(,"t":80}]},
        {"points":[{"x":)" + AString::number(c.x + 20) + R"(,"y":)" + AString::number(c.y) + R"(,"t":0},
                   {"x":)" + AString::number(c.x + 20) + R"(,"y":)" + AString::number(c.y + 40) + R"(,"t":80}]}]})"));
    EXPECT_EQ(result["fingers"].asInt(), 2);
    EXPECT_EQ(result["durationMs"].asInt(), 80);
    EXPECT_NEAR(total.offset.y, 40.f, 2.f);

    EXPECT_EQ(errorCode("input.touch", AJson::fromString("{}")), rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("input.touch", AJson::fromString(R"({"fingers":[{"points":[{"x":1,"y":1,"t":50},{"x":1,"y":1,"t":10}]}]})")),
              rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("input.touch", AJson::fromString(R"({"fingers":[{"finger":1,"points":[{"x":1,"y":1}]},{"finger":1,"points":[{"x":1,"y":1}]}]})")),
              rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("input.touch", AJson { { "gesture", "swirl" }, { "x", 1 }, { "y", 1 } }), rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("input.pointer", AJson { { "x", 1 }, { "y", 1 }, { "action", "fling" } }), rpc_code::INVALID_PARAMS);
}

TEST_F(AgentTest, ScrollIntoView) {
    _<AViewContainer> list = Vertical {};
    for (int i = 0; i < 50; ++i) {
        list->addView(Label { "item {}"_format(i) });
    }
    auto last = _new<ALabel>("the last one");
    list->addView(last);
    auto window = showWindow(AScrollArea::Builder().withContents(list).build() AUI_OVERRIDE_STYLE { FixedSize { 200_px, 100_px } });
    const auto ref = ok("view.find", AJson { { "text", "the last one" } })[0]["ref"].asString();

    auto result = ok("view.scrollIntoView", AJson { { "ref", ref } });
    EXPECT_EQ(result["scrollAreas"].asInt(), 1);
    EXPECT_TRUE(result["visible"].asBool()) << AJson::toString(result);
    EXPECT_EQ(result["rect"]["absY"].asInt(), last->getPositionInWindow().y);

    EXPECT_EQ(errorCode("view.scrollIntoView", AJson::fromString("{}")), rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("view.scrollIntoView", AJson { { "ref", "v0:99999" } }), rpc_code::STALE_REF);
}

TEST_F(AgentTest, WindowSetScale) {
    auto window = showWindow(Label { "hi" });
    const auto systemDpi = window->getDpiRatio();

    auto result = ok("window.setScale", AJson { { "factor", 2 } });
    EXPECT_FLOAT_EQ(float(result["scalingFactor"].asNumber()), 2.f);
    EXPECT_FLOAT_EQ(float(result["dpi"].asNumber()), systemDpi * 2.f);
    EXPECT_FLOAT_EQ(window->getDpiRatio(), systemDpi * 2.f);
    const auto windows = ok("window.list");
    for (const auto& w : windows.asArray()) {
        if (w["title"].asString() == "Agent test") {
            EXPECT_FLOAT_EQ(float(w["scalingFactor"].asNumber()), 2.f);
        }
    }

    auto reset = ok("window.setScale", AJson { { "reset", true } });
    EXPECT_FLOAT_EQ(float(reset["scalingFactor"].asNumber()), 1.f);
    EXPECT_FLOAT_EQ(window->getDpiRatio(), systemDpi);

    EXPECT_EQ(errorCode("window.setScale", AJson { { "factor", 10 } }), rpc_code::INVALID_PARAMS);
    EXPECT_EQ(errorCode("window.setScale", AJson::fromString("{}")), rpc_code::INVALID_PARAMS);
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
    /// Produces the next request from the replies so far; an empty string closes the connection.
    using Script = std::function<AString(const AVector<AString>& replies)>;

    std::atomic<bool> finished = false;
    std::atomic<bool> connected = false;
    AVector<AString> replies;   // read after `finished` only
    std::thread thread;

    WebsocketSession(const AString& url, Script script) {
        thread = std::thread([this, url, script = std::move(script)] {
            try {
                auto ws = _new<AWebsocket>(url);
                // the slots live in ws itself: a strong capture would never let it go
                AObject::connect(ws->connected, ws, [&, ws = ws.get()] {
                    connected = true;
                    ws->writeText(script(replies));
                });
                AObject::connect(ws->received, ws, [&, ws = ws.get()](AByteBufferView payload) {
                    replies << AString::fromUtf8(payload);
                    auto next = script(replies);
                    if (next.empty()) {
                        ws->close();
                    } else {
                        ws->writeText(next);
                    }
                });
                ws->run();
            } catch (const AException&) {
            }
            finished = true;
        });
    }

    /// Sends one request.
    WebsocketSession(const AString& url, AString request)
      : WebsocketSession(url, [request = std::move(request)](const AVector<AString>& replies) {
            return replies.empty() ? request : AString {};
        }) {}

    ~WebsocketSession() { thread.join(); }

    AString reply() const { return replies.empty() ? AString {} : replies.last(); }

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
    const auto url = "wss://127.0.0.1:" + AString::number(agent()->port()) + "/?key=" + KEY;
    WebsocketSession session(url, R"({"jsonrpc":"2.0","id":1,"method":"view.interactive"})");
    session.waitPumping();
    ASSERT_TRUE(session.finished);
    ASSERT_FALSE(session.reply().empty());
    auto response = AJson::fromString(session.reply());
    bool found = false;
    for (const auto& e : response["result"].asArray()) {
        found |= e["label"].asString() == "Click me";
    }
    EXPECT_TRUE(found) << session.reply();
}

TEST_F(AgentTest, UnpairedClientCanNotDoAnything) {
    for (const char* query : { "", "?key=wrong-key-0123456789abcdef" }) {
        WebsocketSession session("wss://127.0.0.1:" + AString::number(agent()->port()) + "/" + query,
                                 R"({"jsonrpc":"2.0","id":1,"method":"view.interactive"})");
        session.waitPumping();
        ASSERT_TRUE(session.finished);
        auto response = AJson::fromString(session.reply());
        EXPECT_EQ(response["error"]["code"].asInt(), rpc_code::UNAUTHORIZED) << session.reply();
        EXPECT_FALSE(response.contains("result"));
    }
}

TEST_F(AgentTest, PairingFlow) {
    const AString newKey = "freshly-generated-key-0123456789";
    auto ids = std::make_shared<int>(0);
    auto rpcText = [ids](const char* method, const AString& params) {
        return "{\"jsonrpc\":\"2.0\",\"id\":" + AString::number(++*ids) + ",\"method\":\"" + method +
               "\",\"params\":" + params + "}";
    };
    WebsocketSession session(
        "wss://127.0.0.1:" + AString::number(agent()->port()) + "/",
        [&, rpcText](const AVector<AString>& replies) -> AString {
            switch (replies.size()) {
                case 0: return rpcText("pair.begin", "{}");
                case 1: return rpcText("pair.confirm", "{\"code\":\"not-it\",\"key\":\"" + newKey + "\"}");
                case 2:
                    return rpcText("pair.confirm", "{\"code\":\"" + AString(agent()->pairing().currentCode()) +
                                                       "\",\"key\":\"" + newKey + "\"}");
                case 3: return rpcText("session.hello", "{}");   // the same connection is authorized now
                default: return {};
            }
        });
    session.waitPumping();
    ASSERT_TRUE(session.finished);
    ASSERT_EQ(session.replies.size(), 4u);
    EXPECT_TRUE(AJson::fromString(session.replies[0])["result"]["pending"].asBool());
    EXPECT_EQ(AJson::fromString(session.replies[1])["error"]["code"].asInt(), rpc_code::UNAUTHORIZED);
    EXPECT_EQ(AJson::fromString(session.replies[1])["error"]["data"]["attemptsLeft"].asInt(), 2);
    EXPECT_TRUE(AJson::fromString(session.replies[2])["result"]["paired"].asBool());
    EXPECT_EQ(AJson::fromString(session.replies[3])["result"]["protocol"].asInt(), 1);

    // later connections just present the key
    EXPECT_TRUE(agent()->pairing().isAuthorized(newKey.toStdString()));
    WebsocketSession again("wss://127.0.0.1:" + AString::number(agent()->port()) + "/?key=" + newKey,
                           R"({"jsonrpc":"2.0","id":1,"method":"session.hello"})");
    again.waitPumping();
    EXPECT_EQ(AJson::fromString(again.reply())["result"]["protocol"].asInt(), 1);
}

TEST_F(AgentTest, PairingCodeIsLimitedToThreeAttempts) {
    auto& pairing = agent()->pairing();
    pairing.begin();
    const AString key = "another-key-0123456789abcdef";
    const auto code = pairing.currentCode();
    const AString wrong = code == "000000" ? AString("111111") : AString("000000");
    for (int i = 0; i < 3; ++i) {
        EXPECT_THROW(pairing.confirm(wrong, key.toStdString()), RpcException);
    }
    // even the right code is void now
    EXPECT_THROW(pairing.confirm(AString(code), key.toStdString()), RpcException);
    EXPECT_FALSE(pairing.isAuthorized(key.toStdString()));
}

TEST_F(AgentTest, AskingForNewCodesIsThrottled) {
    // a fresh instance: the one of the agent is shared with other tests
    aui::remote::Pairing pairing;
    for (int i = 0; i < aui::remote::Pairing::FREE_WASTED_CODES + 2; ++i) {
        pairing.begin();   // each one replaces the previous unused code
    }
    EXPECT_THROW(pairing.begin(), RpcException);
}

namespace {
void pairWith(aui::remote::Pairing& pairing, const std::string& key) {
    pairing.begin();
    pairing.confirm(AString(pairing.currentCode()), key);
}
}   // namespace

TEST_F(AgentTest, PairedKeySurvivesRestart) {
    const auto file = APath::getDefaultPath(APath::TEMP) / "aui-pairing-test" / "trusted.keys";
    file.parent().makeDirs();
    if (file.isRegularFileExists()) {
        file.removeFile();
    }
    const std::string key = "persistent-key-0123456789abcdef";
    {
        aui::remote::Pairing before;
        before.persistTo(file);
        EXPECT_EQ(before.trust(), aui::remote::Pairing::Trust::PERSISTENT);
        pairWith(before, key);
        EXPECT_TRUE(before.isAuthorized(key));
    }
    aui::remote::Pairing after;   // the application has been restarted
    after.persistTo(file);
    EXPECT_TRUE(after.isAuthorized(key));
    EXPECT_FALSE(after.isAuthorized("some-other-key-0123456789abcd"));

    // only a hash is stored
    std::ifstream in(file.toStdString());
    std::stringstream content;
    content << in.rdbuf();
    EXPECT_EQ(content.str().find(key), std::string::npos);
}

TEST_F(AgentTest, SessionPairingIsForgottenOnRestart) {
    const std::string key = "session-key-0123456789abcdef";
    {
        aui::remote::Pairing before;   // no file: trusted until exit
        EXPECT_EQ(before.trust(), aui::remote::Pairing::Trust::SESSION);
        pairWith(before, key);
        EXPECT_TRUE(before.isAuthorized(key));
    }
    aui::remote::Pairing after;
    EXPECT_FALSE(after.isAuthorized(key));
}

TEST_F(AgentTest, AlwaysRequireCodeDistrustsKnownKeys) {
    const std::string key = "always-key-0123456789abcdef";
    aui::remote::Pairing pairing;
    pairing.setAlwaysRequireCode(true);
    EXPECT_EQ(pairing.trust(), aui::remote::Pairing::Trust::CONNECTION);
    pairWith(pairing, key);
    EXPECT_FALSE(pairing.isAuthorized(key));
    pairing.authorize("launch-key-0123456789abcdef");   // launched by the client itself
    EXPECT_TRUE(pairing.isAuthorized("launch-key-0123456789abcdef"));
}

TEST_F(AgentTest, ShortKeyIsRejected) {
    auto& pairing = agent()->pairing();
    pairing.begin();
    EXPECT_THROW(pairing.confirm(AString(pairing.currentCode()), "short"), RpcException);
}

TEST_F(AgentTest, PlainWebsocketIsNotServed) {
    // there is no ws:// mode: the TLS handshake fails, so the client never gets connected
    WebsocketSession session("ws://127.0.0.1:" + AString::number(agent()->port()) + "/?key=" + KEY,
                             R"({"jsonrpc":"2.0","id":1,"method":"session.hello"})");
    session.waitPumping();
    EXPECT_TRUE(session.finished);
    EXPECT_FALSE(session.connected);
    EXPECT_TRUE(session.replies.empty());
}

TEST_F(AgentTest, CertificateFingerprintIsPublished) {
    const auto fingerprint = agent()->certificateFingerprint();
    EXPECT_EQ(fingerprint.length(), 64u);
    for (AChar c : fingerprint) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
}

TEST_F(AgentTest, PresentedCertificateMatchesThePublishedFingerprint) {
    std::thread client([&] {
        ATcpSocket socket(AInet4Address::fromString("127.0.0.1", agent()->port()));
        ATlsTransport transport {
            [&](std::chrono::milliseconds t) { return socket.waitForRead(t); },
            [&](std::span<std::byte> dst) { return socket.read(dst); },
            [&](std::span<const std::byte> src) { socket.write(reinterpret_cast<const char*>(src.data()), src.size()); },
        };
        auto session = ATlsSession::connect(transport);
        ASSERT_TRUE(session->peerFingerprintSha256());
        EXPECT_EQ(ATlsIdentity::toHex(*session->peerFingerprintSha256()), agent()->certificateFingerprint());
    });
    client.join();
}

namespace {

/// Stands for a client: collects what the agent pushes. Sending can be held to emulate a slow connection.
class TestConnection : public RpcConnection {
public:
    bool sendText(const AString& text) override {
        std::unique_lock lock(mMutex);
        ++mSendsStarted;
        mCv.wait(lock, [&] { return !mHeld; });
        mMessages.push_back(AJson::fromString(text));
        return true;
    }

    void hold(bool held) {
        {
            std::lock_guard lock(mMutex);
            mHeld = held;
        }
        mCv.notify_all();
    }

    AVector<AJson> messages() {
        std::lock_guard lock(mMutex);
        return mMessages;
    }

    int sendsStarted() {
        std::lock_guard lock(mMutex);
        return mSendsStarted;
    }

    ~TestConnection() override { hold(false); }

private:
    std::mutex mMutex;
    std::condition_variable mCv;
    AVector<AJson> mMessages;
    int mSendsStarted = 0;
    bool mHeld = false;
};
}   // namespace

class FramesTest : public AgentTest {
protected:
    _<TestConnection> connection = _new<TestConnection>();

    AJson call(const AString& method, AJson params = AJson::fromString("{}")) {
        AJson request {
            { "jsonrpc", "2.0" },
            { "id", 1 },
            { "method", method },
            { "params", std::move(params) },
        };
        AString response;
        agent()->dispatcher().handle(AJson::toString(request), [&](AString r) { response = std::move(r); }, connection);
        pumpUntil([&] { return !response.empty(); });
        EXPECT_FALSE(response.empty()) << "no response to " << method;
        return AJson::fromString(response.empty() ? AString("{}") : response);
    }

    template <typename Condition>
    bool pumpUntil(Condition condition) {
        for (int i = 0; i < 2000 && !condition(); ++i) {
            uitest::frame();
            if (!condition()) {
                std::this_thread::sleep_for(2ms);
            }
        }
        return condition();
    }

    bool waitForFrames(std::size_t count) {
        return pumpUntil([&] { return connection->messages().size() >= count; });
    }

    /// Lets the UI thread settle: whatever would be sent has been sent.
    void settle() {
        for (int i = 0; i < 20; ++i) {
            uitest::frame();
            std::this_thread::sleep_for(2ms);
        }
    }

    static tree_diff::Tree treeOf(const AJson& message, const tree_diff::Tree& base = {}) {
        return tree_diff::apply(tree_diff::diffFromJson(message["params"]["tree"]), base);
    }

    void TearDown() override {
        connection->notifyClosed();
        AgentTest::TearDown();
    }
};

TEST_F(FramesTest, SubscribeSendsFullFrame) {
    auto window = showWindow(Label { "hello frames" });
    EXPECT_TRUE(call("frames.subscribe")["result"]["subscribed"].asBool());
    ASSERT_TRUE(waitForFrames(1));

    const auto message = connection->messages().front();
    EXPECT_FALSE(message.contains("id"));
    EXPECT_EQ(message["method"].asString(), "frames.frame");
    const auto& frame = message["params"];
    EXPECT_EQ(frame["seq"].asLongInt(), 1);
    EXPECT_EQ(frame["windowWidth"].asInt(), window->getWidth());
    EXPECT_EQ(frame["windowHeight"].asInt(), window->getHeight());
    EXPECT_GT(frame["width"].asInt(), 0);
    EXPECT_LE(frame["width"].asInt(), frame["windowWidth"].asInt() * 4);   // downscaled by 0.5 of the device pixels

    const auto jpeg = AByteBuffer::fromBase64String(frame["jpeg"].asString());
    ASSERT_GT(jpeg.size(), 2u);
    EXPECT_EQ(static_cast<unsigned char>(jpeg.data()[0]), 0xff);
    EXPECT_EQ(static_cast<unsigned char>(jpeg.data()[1]), 0xd8);

    EXPECT_TRUE(frame["tree"]["base"].isNull());
    const auto tree = treeOf(message);
    ASSERT_TRUE(tree.nodes.contains(tree.root));
    EXPECT_EQ(tree.nodes.at(tree.root).type.contains("AWindow"), true);
    bool hasLabel = false;
    for (const auto& [ref, node] : tree.nodes) {
        hasLabel |= node.text && *node.text == "hello frames";
    }
    EXPECT_TRUE(hasLabel);
}

TEST_F(FramesTest, ScaleAndQualityAreValidated) {
    auto window = showWindow(Label { "hi" });
    for (const auto& params : { AJson { { "scale", 0.0 } }, AJson { { "scale", 1.5 } }, AJson { { "quality", 0 } },
                                AJson { { "quality", 101 } } }) {
        auto response = call("frames.subscribe", params);
        ASSERT_TRUE(response.contains("error")) << AJson::toString(params);
        EXPECT_EQ(response["error"]["code"].asInt(), rpc_code::INVALID_PARAMS);
    }
    settle();
    EXPECT_TRUE(connection->messages().empty());
}

TEST_F(FramesTest, FullScaleKeepsThePixels) {
    auto window = showWindow(Label { "hi" });
    call("frames.subscribe", AJson { { "scale", 1.0 }, { "quality", 100 } });
    ASSERT_TRUE(waitForFrames(1));
    const auto frame = connection->messages().front()["params"];
    const auto screenshot = call("screenshot")["result"];
    EXPECT_EQ(frame["width"].asInt(), screenshot["width"].asInt());
    EXPECT_EQ(frame["height"].asInt(), screenshot["height"].asInt());
}

TEST_F(FramesTest, OnlyOneFrameInFlight) {
    auto label = _new<ALabel>("zero");
    auto window = showWindow(label);
    connection->hold(true);
    call("frames.subscribe");
    ASSERT_TRUE(pumpUntil([&] { return connection->sendsStarted() == 1; }));

    // renders while the first frame is stuck: nothing may be sent or queued
    for (int i = 0; i < 5; ++i) {
        label->setText("text " + AString::number(i));
        settle();
    }
    EXPECT_EQ(connection->sendsStarted(), 1);

    connection->hold(false);
    ASSERT_TRUE(waitForFrames(2));
    const auto messages = connection->messages();
    // the five renders left no trace: the numbering is gapless
    EXPECT_EQ(messages[1]["params"]["seq"].asLongInt(), 2);

    // the second frame carries the latest state; the first was not acked, so it is a full tree
    EXPECT_TRUE(messages[1]["params"]["tree"]["base"].isNull());
    bool latest = false;
    for (const auto& [ref, node] : treeOf(messages[1]).nodes) {
        latest |= node.text && *node.text == "text 4";
    }
    EXPECT_TRUE(latest);
}

TEST_F(FramesTest, DiffIsAgainstTheLastAckedFrame) {
    // the window renders all the time under UITest, so frames flow continuously: look for the one with the change
    auto hasText = [](const AJson& message, const AString& text) {
        for (const auto& [ref, node] : tree_diff::diffFromJson(message["params"]["tree"]).upsert) {
            if (node.text && *node.text == text) {
                return true;
            }
        }
        return false;
    };
    auto waitForText = [&](const AString& text) -> AOptional<AJson> {
        AOptional<AJson> found;
        pumpUntil([&] {
            for (const auto& message : connection->messages()) {
                if (hasText(message, text)) {
                    found = message;
                    return true;
                }
            }
            return false;
        });
        return found;
    };

    auto label = _new<ALabel>("one");
    auto window = showWindow(Vertical { label, Label { "static" } });
    call("frames.subscribe");
    ASSERT_TRUE(waitForFrames(1));
    const auto tree1 = treeOf(connection->messages()[0]);
    call("frames.ack", AJson { { "seq", 1 } });

    label->setText("two");
    auto frame2 = waitForText("two");
    ASSERT_TRUE(frame2);
    EXPECT_EQ((*frame2)["params"]["tree"]["base"].asLongInt(), 1);
    const auto diff2 = tree_diff::diffFromJson((*frame2)["params"]["tree"]);
    EXPECT_LT(diff2.upsert.size(), tree1.nodes.size());   // not the whole tree

    // frame 2 is not acked (as if it was lost on the way): the next one still goes against frame 1
    label->setText("three");
    auto frame3 = waitForText("three");
    ASSERT_TRUE(frame3);
    EXPECT_GT((*frame3)["params"]["seq"].asLongInt(), (*frame2)["params"]["seq"].asLongInt());
    EXPECT_EQ((*frame3)["params"]["tree"]["base"].asLongInt(), 1);
    const auto tree3 = treeOf(*frame3, tree1);
    bool three = false;
    for (const auto& [ref, node] : tree3.nodes) {
        three |= node.text && *node.text == "three";
    }
    EXPECT_TRUE(three);

    // an ack of a frame that was never sent is ignored; an ack of a real one moves the base
    call("frames.ack", AJson { { "seq", 99999 } });
    const auto seq3 = (*frame3)["params"]["seq"].asLongInt();
    call("frames.ack", AJson { { "seq", seq3 } });
    label->setText("four");
    auto frame4 = waitForText("four");
    ASSERT_TRUE(frame4);
    EXPECT_EQ((*frame4)["params"]["tree"]["base"].asLongInt(), seq3);
    EXPECT_EQ(treeOf(*frame4, tree3).nodes.size(), tree3.nodes.size());
}

TEST_F(FramesTest, TreeKeepsRefsOfViewTree) {
    auto label = _new<ALabel>("a");
    auto window = showWindow(label);
    call("frames.subscribe");
    ASSERT_TRUE(waitForFrames(1));
    const auto tree = treeOf(connection->messages()[0]);
    // refs of the tree are usable with the other methods
    AString labelRef;
    for (const auto& [ref, node] : tree.nodes) {
        if (node.text && *node.text == "a") {
            labelRef = ref;
        }
    }
    ASSERT_FALSE(labelRef.empty());
    auto properties = call("view.properties", AJson { { "ref", labelRef } });
    EXPECT_FALSE(properties.contains("error")) << AJson::toString(properties);
}

TEST_F(FramesTest, OverlayDoesNotLoopAndIsHiddenFromFrames) {
    auto window = showWindow(Label { "hi" });
    RemoteOverlay::enable(*window);
    RemoteOverlay::flagActivity(*window);
    call("frames.subscribe");
    ASSERT_TRUE(waitForFrames(1));
    call("frames.ack", AJson { { "seq", 1 } });
    // the overlay is not hidden for good
    settle();
    EXPECT_TRUE(RemoteOverlay::isShown(*window));
}

TEST_F(FramesTest, UnsubscribeStopsFrames) {
    auto label = _new<ALabel>("a");
    auto window = showWindow(label);
    call("frames.subscribe");
    ASSERT_TRUE(waitForFrames(1));
    settle();
    EXPECT_FALSE(call("frames.unsubscribe").contains("error"));
    settle();   // a frame that was on its way when the unsubscribe came may still arrive
    const auto before = connection->messages().size();
    label->setText("b");
    settle();
    EXPECT_EQ(connection->messages().size(), before);
}

TEST_F(FramesTest, ResubscribeForcesFullTree) {
    auto label = _new<ALabel>("a");
    auto window = showWindow(label);
    call("frames.subscribe");
    ASSERT_TRUE(waitForFrames(1));
    call("frames.ack", AJson { { "seq", 1 } });
    settle();
    const auto before = connection->messages().size();

    call("frames.subscribe", AJson { { "scale", 0.25 } });
    // the first frame of the new subscription: the stream goes on, and frames of the old one may still be arriving
    const auto fullWidth = connection->messages().front()["params"]["width"].asInt();
    const auto isNew = [&](const AJson& m) { return m["params"]["width"].asInt() < fullWidth; };
    ASSERT_TRUE(pumpUntil([&] {
        const auto messages = connection->messages();
        return std::any_of(messages.begin() + before, messages.end(), isNew);
    }));
    const auto messages = connection->messages();
    auto first = std::find_if(messages.begin() + before, messages.end(), isNew);
    const auto frame = (*first)["params"];
    EXPECT_TRUE(frame["tree"]["base"].isNull());
    EXPECT_GT(frame["seq"].asLongInt(), 1);   // numbering goes on
}

TEST_F(FramesTest, ClosedConnectionIsForgotten) {
    auto label = _new<ALabel>("a");
    auto window = showWindow(label);
    call("frames.subscribe");
    ASSERT_TRUE(waitForFrames(1));
    settle();
    connection->notifyClosed();
    settle();
    const auto before = connection->messages().size();
    label->setText("b");
    settle();
    // a connection that is over does not receive anything even though the object is alive
    EXPECT_EQ(connection->messages().size(), before);
}

TEST_F(FramesTest, NeedsAConnection) {
    auto window = showWindow(Label { "hi" });
    EXPECT_EQ(errorCode("frames.subscribe"), rpc_code::NOT_SUPPORTED);
}
