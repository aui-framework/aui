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

#include <optional>
#include <glm/glm.hpp>
#include "gtk_types.h"

class AImage;

/**
 * @brief Xlib helpers for GTK4 running on top of the X11 GDK backend.
 * @details
 * GTK4 deliberately hides window positioning, window icons, pointer warping and similar from the application, as
 * Wayland does not support them. When the GDK backend is X11 these can still be done with Xlib. Every function returns
 * false/nullopt if the current GDK backend is not X11 (i.e., Wayland) or the surface is not available.
 */
namespace aui::gtk4_x11 {

/**
 * @return true if GDK uses X11 backend.
 */
bool isActive();

std::optional<glm::ivec2> windowPosition(gtk4_fake::GdkSurface* surface);
bool windowMoveResize(gtk4_fake::GdkSurface* surface, int x, int y, int width, int height, bool overrideRedirect);
bool windowMoveToCenter(gtk4_fake::GdkSurface* surface, gtk4_fake::GdkSurface* parentSurface);
bool windowSetIcon(gtk4_fake::GdkSurface* surface, const AImage& image);
bool windowSetMaxSize(gtk4_fake::GdkSurface* surface, glm::ivec2 maxSize);

std::optional<glm::ivec2> pointerPosition();
bool warpPointer(glm::ivec2 position);

}   // namespace aui::gtk4_x11
