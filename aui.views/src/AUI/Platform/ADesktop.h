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

#include "AUI/Views.h"
#include <glm/glm.hpp>
#include <AUI/IO/APath.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Image/IDrawable.h>

class ACursor;

class ASurface;

/**
 * @brief Desktop-specific functions.
 * @ingroup views
 * @details
 * These functions are intended to work only on desktop platforms; on mobile platforms functions filled with stubs.
 */
namespace ADesktop {

struct FileExtension {
    AString name;
    AString extension;
};

API_AUI_VIEWS glm::ivec2 getMousePosition();
API_AUI_VIEWS void setMousePos(const glm::ivec2& pos);

enum class SystemSound { QUESTION, ASTERISK };

/**
 * @brief Play system sound asynchronously.
 */
API_AUI_VIEWS void playSystemSound(ADesktop::SystemSound s);

/**
 * @brief Opens native browse for directory dialog.
 * @param parent parent window which is put in the ASurface::blockUserInput state while the dialog is active. Can be
 *               nullptr.
 * @param startingLocation path where the file browser dialog opened initially. Can be empty.
 * @return AFuture returning selected path. If user cancelled the operation, the returned path is empty.
 * @details
 * The future is returned instead of the regular path due to platform limitations on Windows. Never try to call
 * blocking getter since it would cause deadlock. Use AFuture::onValue callback instead.
 */
API_AUI_VIEWS AFuture<APath> browseForDir(ASurface* parent, const APath& startingLocation = {});

/**
 * @brief Opens native browse for file dialog.
 * @param parent parent window which is put in the ASurface::blockUserInput state while the dialog is active. Can be
 *               nullptr.
 * @param startingLocation path where the file browser dialog opened initially. Can be empty.
 * @param extensions extensions.
 * @return AFuture returning selected path. If user cancelled the operation, the returned path is empty.
 * @details
 * The future is returned instead of the regular path due to platform limitations on Windows. Never try to call
 * blocking getter since it would cause deadlock. Use AFuture::onValue callback instead.
 *
 * @specificto{emscripten}
 * The browser's file picker: the chosen file is copied into the in-memory file system (`/tmp/aui-picked/<its name>`)
 * and that path is returned. `startingLocation` is ignored. Call it from an input event handler (a click): browsers
 * open the picker only shortly after a user gesture.
 */
API_AUI_VIEWS AFuture<APath> browseForFile(
    ASurface* parent, const APath& startingLocation = {},
    const AVector<FileExtension>& extensions = { { "All", "*" } });

/**
 * @brief Opens native "save as" dialog.
 * @param parent parent window which is put in the ASurface::blockUserInput state while the dialog is active. Can be
 *               nullptr.
 * @param startingLocation path where the file browser dialog opened initially. Can be empty.
 * @param suggestedName file name the dialog offers. Can be empty.
 * @param extensions extensions.
 * @return AFuture returning the path the user chose; nothing is written there yet. If user cancelled the operation,
 *         the returned path is empty.
 * @details
 * The same rules as for browseForFile apply: never block on the future, use AFuture::onValue.
 *
 * @specificto{emscripten}
 * A page can't pick a path to write to: the returned path is always empty. Use ADesktop::saveFile, it downloads the
 * file.
 */
API_AUI_VIEWS AFuture<APath> browseForSaveFile(
    ASurface* parent, const APath& startingLocation = {}, const AString& suggestedName = {},
    const AVector<FileExtension>& extensions = { { "All", "*" } });

/**
 * @brief Gives a file to the user: asks where to save it ("save as" dialog) and writes `contents` there.
 * @param parent parent window which is put in the ASurface::blockUserInput state while the dialog is active. Can be
 *               nullptr.
 * @param suggestedName file name the dialog offers.
 * @param contents bytes of the file.
 * @param extensions extensions.
 * @return AFuture returning the path written. If user cancelled the operation, the returned path is empty. Fails if the
 *         file can't be written.
 * @details
 * ```cpp
 * ADesktop::saveFile(window, "notes.txt", AByteBuffer::fromString(text), { { "Text", "txt" } })
 *     .onValue([](const APath& path) { if (!path.empty()) ALogger::info("Notes") << "saved to " << path; });
 * ```
 *
 * @specificto{emscripten}
 * The browser downloads the file as `suggestedName` (to the downloads folder, or where the user picks, depending on
 * the browser settings). The returned path is `suggestedName`; the browser does not tell whether the user cancelled.
 */
API_AUI_VIEWS AFuture<APath> saveFile(
    ASurface* parent, const AString& suggestedName, AByteBuffer contents,
    const AVector<FileExtension>& extensions = { { "All", "*" } });

API_AUI_VIEWS _<IDrawable> iconOfFile(const APath& file);
}   // namespace ADesktop
