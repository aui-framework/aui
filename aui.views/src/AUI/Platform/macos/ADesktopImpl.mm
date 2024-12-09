/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/ARaiiHelper.h>

#include <AppKit/AppKit.h>
#include <Availability.h>

#if !defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || !defined(__MAC_11_0) || __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_11_0
#define AUI_MACOS_SUPPORTS_CONTENTTYPES 0
#else
#define AUI_MACOS_SUPPORTS_CONTENTTYPES 1
#endif

#if AUI_MACOS_SUPPORTS_CONTENTTYPES == 1
#include <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#endif

glm::ivec2 ADesktop::getMousePosition() {
    @autoreleasepool {
        NSPoint mouseLocation = [NSEvent mouseLocation];

        // Convert screen coordinates to flipped coordinates
        NSScreen* mainScreen = [NSScreen mainScreen];
        mouseLocation.y = mainScreen.frame.size.height - mouseLocation.y;

        return { mouseLocation.x, mouseLocation.y };
    }
}

void ADesktop::setMousePos(const glm::ivec2& pos) {}

AFuture<APath> ADesktop::browseForDir(AWindowBase* parent, const APath& startingLocation) {
    return async { return APath(); };
}

AFuture<APath>
ADesktop::browseForFile(AWindowBase* parent, const APath& startingLocation, const AVector<FileExtension>& extensions) {
    return async { return APath(); };
}

_<IDrawable> ADesktop::iconOfFile(const APath& file) { return nullptr; }

void ADesktop::playSystemSound(ADesktop::SystemSound s) {
    // unsupported
}
