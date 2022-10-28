/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include "AUI/Views.h"
#include <glm/glm.hpp>
#include <AUI/IO/APath.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Image/IDrawable.h>

enum class ACursor;

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
