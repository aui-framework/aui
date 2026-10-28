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

#include <chrono>

#include <glm/glm.hpp>

#include "AUI/Event/APointerIndex.h"
#include "AUI/Platform/AWindow.h"

namespace aui::remote {

/**
 * @brief Tells the user sitting in front of the application that it is being controlled remotely.
 * @details
 * Per window, drawn on top of the contents at the end of every frame (see AWindow::postRender):
 *
 * - a pink banner at the top edge, for a few seconds after every command;
 * - a pink pointer (an arrow for a mouse, a circle for a finger) where the last pointer action happened, for a few
 *   seconds, with a short ripple on click.
 *
 * Must be used on the UI thread only.
 */
class RemoteOverlay {
public:
    /**
     * @brief Turns the overlay on for the window. Does nothing if it is already on.
     */
    static void enable(AWindow& window);

    [[nodiscard]]
    static bool isEnabled(AWindow& window);

    /**
     * @brief Shows the banner: a remote command has just arrived. Does nothing unless the overlay is enabled.
     */
    static void flagActivity(AWindow& window);

    /**
     * @brief Shows the pointer at the position (px). Does nothing unless the overlay is enabled.
     * @param click plays the ripple
     * @param held the button is pressed and not released yet: the banner and the pointer stay until the next call
     *        with `held` false, which starts their timeout
     */
    static void showPointer(AWindow& window, glm::vec2 position, APointerIndex pointer, bool click, bool held);

    /**
     * @return true if the overlay draws anything on the window right now (a banner, a pointer or a ripple)
     */
    [[nodiscard]]
    static bool isShown(AWindow& window);

    /**
     * @brief Hides the overlay while `hidden` is true, so that the next frame is clean. Used by screenshots.
     */
    static void setHidden(AWindow& window, bool hidden);

    ~RemoteOverlay();

private:
    explicit RemoteOverlay(AWindow& window);
    static RemoteOverlay* of(AWindow& window);

    /// Like a plain age of the event, but a held pointer never gets old.
    AOptional<std::chrono::steady_clock::duration> visibleAge(std::chrono::steady_clock::time_point time,
                                                              std::chrono::steady_clock::duration lifetime) const;
    void draw(IRenderer& renderer);
    void drawBanner(IRenderer& renderer);
    void drawPointer(IRenderer& renderer);

    AWindow& mWindow;
    bool mHidden = false;
    std::chrono::steady_clock::time_point mActivityTime {};
    std::chrono::steady_clock::time_point mPointerTime {};
    std::chrono::steady_clock::time_point mClickTime {};
    glm::vec2 mPointer { 0 };
    bool mFinger = false;
    bool mHeld = false;
};

}   // namespace aui::remote
