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
#include <functional>

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
 * - a pink pointer (an arrow for the mouse, a circle per finger) where the last action of that pointer happened, for a
 *   few seconds, with a short ripple on click. Every finger has its own circle, so a multi-touch gesture shows all of
 *   them; all mouse buttons share the arrow.
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
     * @param held the button (finger) is pressed and not released yet: the banner and the pointer stay until the next
     *        call for the same pointer with `held` false, which starts their timeout
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

    /**
     * @brief Registers a callback that runs on every frame of the window right before the overlay is drawn, i.e. when
     *        the frame is complete and still clean. Does nothing unless the overlay is enabled.
     * @param owner identifies the callback for removeBeforeDraw
     */
    static void addBeforeDraw(AWindow& window, const void* owner, std::function<void()> callback);

    static void removeBeforeDraw(AWindow& window, const void* owner);

    ~RemoteOverlay();

private:
    explicit RemoteOverlay(AWindow& window);
    static RemoteOverlay* of(AWindow& window);

    struct Marker {
        std::chrono::steady_clock::time_point time {};
        std::chrono::steady_clock::time_point clickTime {};
        glm::vec2 position { 0 };
        bool finger = false;
        bool held = false;
    };

    [[nodiscard]] bool anyHeld() const;

    /// Like a plain age of the event, but never gets old while `held`.
    static AOptional<std::chrono::steady_clock::duration> visibleAge(std::chrono::steady_clock::time_point time,
                                                                     std::chrono::steady_clock::duration lifetime,
                                                                     bool held);
    void draw(IRenderer& renderer);
    void drawBanner(IRenderer& renderer);
    void drawMarker(IRenderer& renderer, const Marker& marker);

    AWindow& mWindow;
    bool mHidden = false;
    AVector<std::pair<const void*, std::function<void()>>> mBeforeDraw;
    std::chrono::steady_clock::time_point mActivityTime {};
    /// The mouse is -1, fingers are their indices.
    AMap<int, Marker> mMarkers;
};

}   // namespace aui::remote
