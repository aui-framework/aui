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

#include <AUI/View/AViewContainer.h>

/**
 * @brief A progress bar.
 * @ingroup useful_views
 * @details
 * A progress bar is used to express a long operation (i.e. file copy) with known progress and reassure the user that
 * application is still running.
 */
class API_AUI_VIEWS AProgressBar: public AViewContainer {
public:
    class Inner: public AView {
    public:
        ~Inner() override;
    };
    ~AProgressBar() override;

    /**
     * Set progress bar value.
     * @param value progress value, where `0.0f` = 0%, `1.0f` = 100%
     */
    void setValue(float value) {
        mValue = value;
        updateInnerWidth();
        redraw();
    }

public:
    AProgressBar();

    void setSize(int width, int height) override;
    void render() override;

private:
    float mValue = 0.f;
    void updateInnerWidth();
    _<Inner> mInner;
};


