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
// Created by alex2 on 23.10.2020.
//

#pragma once


#include <AUI/Image/IDrawable.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Image/AImage.h>
#include <AUI/Render/IRenderer.h>

class AImageDrawable: public IDrawable {
private:
    glm::ivec2 mSize;
    std::variant<_<ITexture>, _<AImage>> mStorage;

public:
    explicit AImageDrawable(_<AImage> image);
    virtual ~AImageDrawable();

    void draw(IRenderer& render, const IDrawable::Params& params) override;

    AImage rasterize(glm::ivec2 imageSize) override;

    glm::ivec2 getSizeHint() override;
};


