/**
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 06.12.2020.
//

#pragma once

#include <AUI/Util/LayoutDirection.h>
#include "AViewContainer.h"
#include "ASpacer.h"

class AScrollbarButton: public AView {
public:
    AScrollbarButton() {

    }
};
class AScrollbarHandle: public AView {
public:
    AScrollbarHandle() {

    }

    int getMinimumWidth() override {
        return 0;
    }

    int getMinimumHeight() override {
        return 0;
    }
};
class AScrollbarOffsetSpacer: public ASpacer {
public:
    AScrollbarOffsetSpacer(): ASpacer(0, 0) {

    }

    int getMinimumWidth() override {
        return 0;
    }

    int getMinimumHeight() override {
        return 0;
    }
};
class API_AUI_VIEWS AScrollbar: public AViewContainer {
private:
    LayoutDirection mDirection;
    _<ASpacer> mOffsetSpacer;
    _<AScrollbarHandle> mHandle;
    _<AScrollbarButton> mForwardButton;
    _<AScrollbarButton> mBackwardButton;

    size_t mViewportSize = 0, mFullSize = 0;
    int mCurrentScroll = 0;

    void setOffset(size_t o);

public:

    explicit AScrollbar(LayoutDirection direction = LayoutDirection::VERTICAL);

    void setScrollDimensions(size_t viewportSize, size_t fullSize);

    void updateScrollHandleSize();
    void setScroll(int scroll);
    void onMouseWheel(glm::ivec2 pos, int delta) override;

signals:
    emits<int> scrolled;
};


