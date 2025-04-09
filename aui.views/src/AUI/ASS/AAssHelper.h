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

//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Common/AVector.h>
#include <optional>

#include "Property/BackgroundCropping.h"
#include "Property/BackgroundImage.h"
#include "Rule.h"

class IDrawable;

/**
 * @brief Remember, ASS is not a butt. ASS is Aui Style Sheets
 */
class AAssHelper: public AObject {
    friend class AView;

private:
    AVector<ass::Rule> mPossiblyApplicableRules;

public:
    void onInvalidateFullAss() {
        emit invalidateFullAss();
    }
    void onInvalidateStateAss() {
        emit invalidateStateAss();
    }

    [[nodiscard]]
    const AVector<ass::Rule>& getPossiblyApplicableRules() const {
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



