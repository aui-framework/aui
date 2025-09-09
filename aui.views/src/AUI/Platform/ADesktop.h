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
#include <AUI/Thread/AFuture.h>
#include <AUI/Image/IDrawable.h>

class ACursor;

class AWindowBase;

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
 * @param parent parent window which is put in the AWindowBase::blockUserInput state while the dialog is active. Can be
 *               nullptr.
 * @param startingLocation path where the file browser dialog opened initially. Can be empty.
 * @return AFuture returning selected path. If user cancelled the operation, the returned path is empty.
 * @details
 * The future is returned instead of the regular path due to platform limitations on Windows. Never try to call
 * blocking getter since it would cause deadlock. Use AFuture::onSuccess callback instead.
 */
API_AUI_VIEWS AFuture<APath> browseForDir(AWindowBase* parent, const APath& startingLocation = {});

/**
 * @brief Opens native browse for file dialog.
 * @param parent parent window which is put in the AWindowBase::blockUserInput state while the dialog is active. Can be
 *               nullptr.
 * @param startingLocation path where the file browser dialog opened initially. Can be empty.
 * @param extensions extensions.
 * @return AFuture returning selected path. If user cancelled the operation, the returned path is empty.
 * @details
 * The future is returned instead of the regular path due to platform limitations on Windows. Never try to call
 * blocking getter since it would cause deadlock. Use AFuture::onSuccess callback instead.
 */
API_AUI_VIEWS AFuture<APath> browseForFile(
    AWindowBase* parent, const APath& startingLocation = {},
    const AVector<FileExtension>& extensions = { { "All", "*" } });

API_AUI_VIEWS _<IDrawable> iconOfFile(const APath& file);
}   // namespace ADesktop
