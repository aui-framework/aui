/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by dervisdev on 1/12/2023.
//
#pragma once

#include "IDrawable.h"
#include "AUI/Image/IAnimatedImageFactory.h"
#include "AUI/Render/ARender.h"
#include "AUI/Common/ASignal.h"

class API_AUI_VIEWS AAnimatedDrawable : public IDrawable, public AObject {
private:
    _<IAnimatedImageFactory> mFactory;
    _<ITexture> mTexture;
public:
    explicit AAnimatedDrawable(_<IAnimatedImageFactory> factory);
    ~AAnimatedDrawable() override = default;

    void draw(const Params &params) override;
    glm::ivec2 getSizeHint() override;

signals:
    emits<> animationFinished;
};

