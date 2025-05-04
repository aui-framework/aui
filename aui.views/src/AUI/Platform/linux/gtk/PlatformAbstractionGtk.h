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

#include <gtk/gtk.h>
#include "AUI/Platform/linux/IPlatformAbstraction.h"
#include "AUI/Platform/linux/AGlibPtr.h"

class PlatformAbstractionGtk: public IPlatformAbstraction {
public:
    static GtkWindow*& nativeHandle(AWindow& window) {
        return reinterpret_cast<GtkWindow*&>(IPlatformAbstraction::nativeHandle(window));
    }

    PlatformAbstractionGtk();
    ~PlatformAbstractionGtk() override;
    _<ACursor::Custom> createCustomCursor(AImageView image) override;
    void applyNativeCursor(const ACursor &cursor, AWindow *pWindow) override;
    void copyToClipboard(const AString &text) override;
    AString pasteFromClipboard() override;
    glm::ivec2 desktopGetMousePosition() override;
    void desktopSetMousePosition(glm::ivec2 pos) override;
    float platformGetDpiRatio() override;
    AInput::Key inputFromNative(int k) override;
    int inputToNative(AInput::Key key) override;
    bool inputIsKeyDown(AInput::Key k) override;
    void windowSetStyle(AWindow &window, WindowStyle ws) override;
    float windowFetchDpiFromSystem(AWindow &window) override;
    void windowRestore(AWindow &window) override;
    void windowMinimize(AWindow &window) override;
    bool windowIsMinimized(AWindow &window) const override;
    bool windowIsMaximized(AWindow &window) const override;
    void windowMaximize(AWindow &window) override;
    glm::ivec2 windowGetPosition(AWindow &window) const override;
    void windowFlagRedraw(AWindow &window) override;
    void windowShow(AWindow &window) override;
    void windowSetSize(AWindow &window, glm::ivec2 size) override;
    void windowSetGeometry(AWindow &window, int x, int y, int width, int height) override;
    void windowSetIcon(AWindow &window, const AImage &image) override;
    void windowHide(AWindow &window) override;
    void windowManagerNotifyProcessMessages() override;
    void windowManagerLoop() override;
    void windowBlockUserInput(AWindow &window, bool blockUserInput) override;
    void windowAllowDragNDrop(AWindow &window) override;
    void windowShowTouchscreenKeyboardImpl(AWindow &window) override;
    void windowHideTouchscreenKeyboardImpl(AWindow &window) override;
    void windowMoveToCenter(AWindow &window) override;
    void windowQuit(AWindow &window) override;
    void windowAnnounceMinMaxSize(AWindow &window) override;
    void windowManagerInitNativeWindow(const IRenderingContext::Init &init) override;
    float windowGetDpiRatio(AWindow &window) override;

    static AInput::Key inputFromNative2(unsigned key);

private:
    AGlibPtr<GApplication> mApplication;
    GMainContext* mMainContext;
};
