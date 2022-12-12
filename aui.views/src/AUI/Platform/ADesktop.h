// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "AUI/Views.h"
#include <glm/glm.hpp>
#include <AUI/IO/APath.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Image/IDrawable.h>

class ACursor;

class ABaseWindow;


namespace ADesktop
{
    struct FileExtension {
        AString name;
        AString extension;
    };

	API_AUI_VIEWS glm::ivec2 getMousePosition();
	API_AUI_VIEWS void setMousePos(const glm::ivec2& pos);

    /**
     * @brief Opens native browse for directory dialog.
     * @param parent parent window which is put in the ABaseWindow::blockUserInput state while the dialog is active. Can be
     *               nullptr.
     * @param startingLocation path where the file browser dialog opened initially. Can be empty.
     * @return AFuture returning selected path. If user cancelled the operation, the returned path is empty.
     * @note
     * The future is returned instead of the regular path due to platform limitations on Windows. Never try to call
     * blocking getter since it would cause deadlock. Use AFuture::onSuccess callback instead.
     */
    API_AUI_VIEWS AFuture<APath> browseForDir(ABaseWindow* parent,
                                              const APath& startingLocation = {});

    /**
     * @brief Opens native browse for file dialog.
     * @param parent parent window which is put in the ABaseWindow::blockUserInput state while the dialog is active. Can be
     *               nullptr.
     * @param startingLocation path where the file browser dialog opened initially. Can be empty.
     * @param extensions extensions.
     * @return AFuture returning selected path. If user cancelled the operation, the returned path is empty.
     * @note
     * The future is returned instead of the regular path due to platform limitations on Windows. Never try to call
     * blocking getter since it would cause deadlock. Use AFuture::onSuccess callback instead.
     */
	API_AUI_VIEWS AFuture<APath> browseForFile(ABaseWindow* parent,
                                               const APath& startingLocation = {},
                                               const AVector<FileExtension>& extensions = { {"All", "*"} });
	API_AUI_VIEWS void openUrl(const AString& url);

    API_AUI_VIEWS _<IDrawable> iconOfFile(const APath& file);
}
