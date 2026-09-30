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

#include "AApplication.h"
#include <cstdlib>
#include <AUI/AppInfo.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Thread/AEventLoop.h>
#include <AUI/Thread/AThread.h>
#include <AUI/Util/kAUI.h>

static constexpr auto LOG_TAG = "AApplication";

namespace {
/**
 * AEventLoop that stops as soon as the application is not held anymore.
 */
class HoldAwareEventLoop : public AEventLoop {
public:
    void loop() override {
        while (AApplication::inst().isHeld()) {
            iteration();
        }
    }
};
}   // namespace

AApplication& AApplication::inst() {
    static AApplication app;
    return app;
}

AApplication::AApplication() {
    setThread(AThread::main());
}

AApplication::Hold::Hold() { ++AApplication::inst().mHoldCount; }

AApplication::Hold::~Hold() {
    --AApplication::inst().mHoldCount;
    AApplication::inst().onHoldReleased();
}

_<AApplication::Hold> AApplication::hold() { return _new<Hold>(); }

APath AApplication::dataDir() const {
    APath base;
#if AUI_PLATFORM_LINUX
    if (auto xdg = getenv("XDG_DATA_HOME"); xdg && *xdg == '/') {
        base = xdg;
    } else {
        base = APath::getDefaultPath(APath::APPDATA);
    }
#else
    base = APath::getDefaultPath(APath::APPDATA);
#endif
    AString id = aui::app_info::app_id;
    if (id == "com.unknown.aui-application") {
        id = aui::app_info::name;
    }
    if (id.empty() || id == "unknown") {
        throw AException(
            "AApplication::dataDir: app id is not set. Specify ID or NAME in aui_app(), otherwise unrelated AUI "
            "applications would share the same data directory.");
    }
    auto dir = base / id;
    dir.makeDirs();
    return dir;
}

void AApplication::onHoldReleased() {
    // wake up the main loop so it can re-evaluate isHeld().
    AThread::main()->enqueue([] {});
}

void AApplication::quit(AOptional<int> exitCode) {
    if (exitCode) {
        mExitCode = exitCode;
    }
    mQuitRequested = true;
    AThread::main()->enqueue([] {});
}

bool AApplication::requestSingleInstanceLock(AOptional<AString> key) {
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS || AUI_PLATFORM_EMSCRIPTEN
    return true;
#else
    if (mSingleInstance) {
        return true;
    }
    auto k = key.valueOr(aui::app_info::app_id);
    if (!key && k == "com.unknown.aui-application") {
        throw AException(
            "AApplication::requestSingleInstanceLock: app id is not set. Specify ID in aui_app() or pass the key "
            "explicitly, otherwise unrelated AUI applications would share the same lock.");
    }
    mSingleInstance = ASingleInstance::acquire(k, [this](AActivation activation) {
        // called from a background thread
        getThread()->enqueue([this, activation = std::move(activation)]() mutable {
            ALogger::info(LOG_TAG) << "Activated by another instance: " << activation.args;
            emit activated(std::move(activation));
        });
    });
    return mSingleInstance != nullptr;
#endif
}

IEventLoop& aui::detail::application::fallbackEventLoop() {
    static HoldAwareEventLoop loop;
    return loop;
}

int aui::detail::application::runMainLoopIfHeld(int entryExitCode) {
    auto& app = AApplication::inst();
    if (auto el = AThread::current()->getCurrentEventLoop()) {
        // aui.views' AWindowManager (or a custom event loop); it checks AApplication::isHeld() by itself.
        el->loop();
    } else if (app.isHeld()) {
        // hold() without any windowing (i.e., a daemon made with aui.core only).
        IEventLoop::Handle h(&fallbackEventLoop());
        fallbackEventLoop().loop();
    }
    return app.mExitCode.valueOr(entryExitCode);
}
