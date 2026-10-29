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

#include "AUI/Platform/AMessageBox.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/View/AText.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
// [ARemoteDebug_include]
#if AUI_DEBUG && __has_include(<AUI/Remote/ARemoteDebug.h>)
#include <AUI/Remote/ARemoteDebug.h>
#define MY_APP_REMOTE_DEBUG 1
#endif
// [ARemoteDebug_include]

struct State {
    AProperty<int> value = 0;
};

using namespace ass;
using namespace declarative;

static _<AView> link(const AString& url) {
    return Label { url } AUI_OVERRIDE_STYLE {
        TextColor { AColor::BLUE },
        BorderBottom { 1_px, AColor::BLUE },
        ACursor::POINTER,
    } AUI_LET { AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [url] { APlatform::openUrl(url); }); };
}

AUI_ENTRY {
    // [ARemoteDebug_start]
#if MY_APP_REMOTE_DEBUG
    // opt-in remote debugging: waits for aui-remote (or any JSON-RPC client) on a loopback port.
    auto debug = ARemoteDebug::start(ARemoteDebug::Options::fromEnvironment());

    // additional feature: you can register your own functions to be called by aui-remote GUI, CLI or an AI agent
    debug->registerFunction("show_window", "Show a modal window.", [](const AJson& args) -> AFuture<AJson> {
        AMessageBox::show(nullptr, "Remote window", AJson::toString(args));
        co_return AJson::Object { { "ok", true } };
    });
#endif
    // [ARemoteDebug_start]

    auto window = _new<AWindow>("Remote debug", 600_dp, 300_dp);
    auto state = _new<State>();
    window->setContents(Centered {
      Vertical {
        Label { AUI_REACT("Value: {}"_format(*state->value)) },
        Button {
          .content = Label { "Click me" },
          .onClick = [state]() mutable { state->value += 1; },
        },
        AText::fromItems({ "Use aui-remote (", link("https://aui-framework.github.io/master/agentic-development"), ") to connect to this application." })
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
    });
    window->show();
    return 0;
}
