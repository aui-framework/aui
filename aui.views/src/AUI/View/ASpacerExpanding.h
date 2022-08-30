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
#include "AView.h"
#include <AUI/Util/Declarative.h>

/**
 * @brief Expanding view which is useful in UI building.
 * @ingroup useful_views
 * @details
 *
 * AExpandingSpacer is expanding blank view which tries acquire space as much as possible.
 */
class API_AUI_VIEWS ASpacerExpanding: public AView
{
private:
    glm::ivec2 mMinimumSize = {20, 20};

public:
	ASpacerExpanding(int w = 4, int h = 4)
	{
		setExpanding({ w, h });
	}
	virtual ~ASpacerExpanding() = default;

    void setMinimumSize(const glm::ivec2& minimumSize) {
        mMinimumSize = minimumSize;
    }

    bool consumesClick(const glm::ivec2& pos) override;

    int getContentMinimumWidth(ALayoutDirection layout) override;
    int getContentMinimumHeight(ALayoutDirection layout) override;
};


namespace declarative {
    using SpacerExpanding = aui::ui_building::view<ASpacerExpanding>;
}