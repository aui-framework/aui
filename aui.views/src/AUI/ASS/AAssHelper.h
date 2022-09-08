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

//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Common/AVector.h>
#include <optional>

#include "Declaration/BackgroundCropping.h"
#include "Declaration/BackgroundImage.h"

struct Rule;
class IDrawable;

/**
 * @brief Remember, ASS is not a butt. ASS is Aui Style Sheets
 */
class AAssHelper: public AObject {
    friend class AView;

private:
    AVector<const Rule*> mPossiblyApplicableRules;

public:
    void onInvalidateFullAss() {
        emit invalidateFullAss();
    }
    void onInvalidateStateAss() {
        emit invalidateStateAss();
    }

    [[nodiscard]]
    const AVector<const Rule*>& getPossiblyApplicableRules() const {
        return mPossiblyApplicableRules;
    }

    struct State {
        ass::BackgroundImage backgroundUrl;
        ass::BackgroundCropping backgroundCropping;
        AOptional<_<IDrawable>> backgroundImage;
        ImageRendering imageRendering = ImageRendering::PIXELATED;
    } state;
signals:
    emits<> invalidateFullAss;
    emits<> invalidateStateAss;
};



