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

#include <gio/gio.h>
#include "gtk_types.h"
#include "AUI/Platform/linux/IPlatformAbstraction.h"
#include "AUI/Platform/linux/AGlibPtr.h"

class RenderingContextGtk;

class PlatformAbstractionGtk: public IPlatformAbstraction {
public:
    static aui::gtk4_fake::GtkWindow*& nativeHandle(AWindow& window) {
        return reinterpret_cast<aui::gtk4_fake::GtkWindow*&>(IPlatformAbstraction::nativeHandle(window));
    }

    /**
     * @brief Native handle of the window as a widget. Either GtkWindow or, for popups, GtkPopover.
     */
    static aui::gtk4_fake::GtkWidget* gtkWidget(AWindow& window) {
        return reinterpret_cast<aui::gtk4_fake::GtkWidget*>(IPlatformAbstraction::nativeHandle(window));
    }

    /**
     * @brief GtkWindow of the window; nullptr if the window is a popup.
     */
    static aui::gtk4_fake::GtkWindow* nativeWindow(AWindow& window);

    /**
     * @brief Whether the window is displayed as GtkPopover.
     * @details
     * Wayland clients can't position toplevels, but can position popups (xdg_popup). GTK4 exposes them only as
     * GtkPopover, so popup AUI windows (AWindow::isPopup) are hosted in a GtkPopover on Wayland.
     */
    static bool isPopup(AWindow& window);

    PlatformAbstractionGtk();
    ~PlatformAbstractionGtk() override;
    void init() override;
    _<ACursor::Custom> createCustomCursor(AImageView image) override;
    void applyNativeCursor(const ACursor &cursor, AWindow *pWindow) override;
    void setClipboardText(const AString &text) override;
    AString getClipboardText() override;
    glm::ivec2 desktopGetMousePosition() override;
    void desktopSetMousePosition(glm::ivec2 pos) override;
    AInput::Key inputFromNative(int k) override;
    int inputToNative(AInput::Key key) override;
    bool inputIsKeyDown(AInput::Key k) override;
    void windowSetTitle(AWindow &window, const AString& title) override;
    void windowSetResizable(AWindow &window, bool resizable) override;
    bool windowIsResizable(AWindow &window) const override;
    float windowFetchDpiFromSystem(AWindow &window) override;
    void windowRestore(AWindow &window) override;
    void windowMinimize(AWindow &window) override;
    bool windowIsMinimized(AWindow &window) const override;
    bool windowIsMaximized(AWindow &window) const override;
    void windowMaximize(AWindow &window) override;
    glm::ivec2 windowGetPosition(AWindow &window) const override;
    void windowFlagRedraw(AWindow &window) override;
    void windowShow(AWindow &window) override;    void windowActivate(AWindow& window, const AString& activationToken) override;    void windowSetSize(AWindow &window, glm::ivec2 size) override;
    void windowSetGeometry(AWindow &window, int x, int y, int width, int height) override;
    void windowSetIcon(AWindow &window, const AImage &image) override;
    void windowHide(AWindow &window) override;
    void windowManagerNotifyProcessMessages() override;
    void windowManagerLoop() override;
    void windowManagerIteration();
    void windowBlockUserInput(AWindow &window, bool blockUserInput) override;
    void windowAllowDragNDrop(AWindow &window) override;
    void windowShowTouchscreenKeyboardImpl(AWindow &window) override;
    void windowHideTouchscreenKeyboardImpl(AWindow &window) override;
    void windowMoveToCenter(AWindow &window) override;
    void windowAnnounceMinMaxSize(AWindow &window) override;
    void windowBeginMove(AWindow &window) override;
    void windowBeginResize(AWindow &window, ResizeEdge edge) override;
    bool windowHasNativeResizeBorders(AWindow &window) override;
    bool windowHasNativeModality() const override { return true; }

    /**
     * @brief The latest button press; required by the window manager to start interactive move.
     */
    struct LastButtonPress {
        aui::gtk4_fake::GdkDevice* device = nullptr;
        unsigned button = 0;
        double x = 0;
        double y = 0;
        uint32_t time = 0;
    } lastButtonPress;
    void windowManagerInitNativeWindow(const IRenderingContext::Init &init) override;
    float windowGetDpiRatio(AWindow &window) override;
    AMessageBox::ResultButton messageBoxShow(
        AWindow *parent, const AString &title, const AString &message, AMessageBox::Icon icon,
        AMessageBox::Button b) override;

    static AInput::Key inputFromNative2(unsigned key);

protected:
    static constexpr auto POPUP_KEY = "aui-popup";
    AGlibPtr<GApplication> mApplication;
    aui::gtk4_fake::GtkWidget* windowManagerInitGtkBox(const IRenderingContext::Init &init) const;
    void windowManagerInitCommon(const IRenderingContext::Init &init, aui::gtk4_fake::GtkWindow* window);

    /**
     * @brief Hides disabled window buttons by adjusting titlebar's decoration layout.
     */
    static void gtkApplyButtons(aui::gtk4_fake::GtkWidget* titlebar, const AWindow::Init::Buttons& buttons);

    /**
     * @brief Creates popover-hosted window if the window is a popup and the display server needs it.
     * @return true if the window was created.
     */
    bool windowManagerInitPopup(const IRenderingContext::Init &init);
    void gtkSetWindowClassHint(RenderingContextGtk* context, aui::gtk4_fake::GtkWindow* gtkWindow);

    /**
     * @brief Titlebar widget which visibility is controlled by AWindow::Init::decoration.
     * @details
     * Present only if the window was created with client-side decorations (i.e., titlebar was installed before the
     * window got realized).
     */
    static aui::gtk4_fake::GtkWidget* gtkTitlebar(aui::gtk4_fake::GtkWindow* window);
    static void gtkSetTitlebar(aui::gtk4_fake::GtkWindow* window, aui::gtk4_fake::GtkWidget* titlebar);

private:
    static constexpr auto TITLEBAR_KEY = "aui-titlebar";
    void popupSetGeometry(AWindow &window, int x, int y, int width, int height);
    GMainContext* mMainContext;
    
    void setupDpiChangeMonitoring();
};
