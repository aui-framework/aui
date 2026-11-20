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
#include <AUI/IO/AFileOutputStream.h>

#if !AUI_PLATFORM_EMSCRIPTEN
// the browser downloads the file instead: emscripten/ADesktopImpl.cpp
AFuture<APath> ADesktop::saveFile(
    ASurface* parent, const AString& suggestedName, AByteBuffer contents, const AVector<FileExtension>& extensions) {
    return browseForSaveFile(parent, {}, suggestedName, extensions)
        .then([contents = std::move(contents)](const APath& path) {
            if (!path.empty()) {
                AFileOutputStream(path).write(contents.data(), contents.size());
            }
            return path;
        });
}
#endif
