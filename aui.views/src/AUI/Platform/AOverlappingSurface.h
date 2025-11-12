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


#include <AUI/View/AViewContainer.h>

class ASurface;

class API_AUI_VIEWS AOverlappingSurface: public AViewContainer {
    friend class ASurface;
private:
    ASurface* mParentWindow;
    bool mCloseOnClick;

public:
    AOverlappingSurface() = default;

    virtual ~AOverlappingSurface() = default;
    void close();

    bool isCloseOnClick() const {
        return mCloseOnClick;
    }

    void setCloseOnClick(bool closeOnClick) {
        mCloseOnClick = closeOnClick;
    }

    ASurface* getParentWindow() const {
        return mParentWindow;
    }

    virtual void setOverlappingSurfacePosition(glm::ivec2 position) = 0;
    virtual void setOverlappingSurfaceSize(glm::ivec2 size) = 0;
};