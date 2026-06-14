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

#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/ARaiiHelper.h>

glm::ivec2 ADesktop::getMousePosition() {
    return {};
}

void ADesktop::setMousePos(const glm::ivec2 &pos) {

}

AFuture<APath> ADesktop::browseForDir(ASurface* parent,
                                      const APath& startingLocation)  {
    return AUI_THREADPOOL {
        return APath();
    };
}

AFuture<APath>
ADesktop::browseForFile(ASurface* parent,
                        const APath& startingLocation,
                        const AVector<FileExtension>& extensions) {
    return AUI_THREADPOOL {
        return APath();
    };
}


_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    return nullptr;
}

void ADesktop::playSystemSound(ADesktop::SystemSound s)
{
    // unsupported
}