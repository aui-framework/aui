// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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



