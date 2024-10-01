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

#pragma once
#include <functional>
#include <AUI/Common/SharedPtrTypes.h>

#include "AUI/Common/ADeque.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Image/IImageFactory.h"
#include "AUI/Render/ARender.h"


class API_AUI_VIEWS AVectorDrawable: public IDrawable
{
private:
    struct Pair {
        uint64_t key;
        ARender::Texture texture;
    };

    ADeque<Pair> mRasterized;
    _<IImageFactory> mFactory;
public:
    explicit AVectorDrawable(_<IImageFactory> factory): mFactory(std::move(factory)) {}
    ~AVectorDrawable();

	void draw(IRenderer& render, const IDrawable::Params& params) override;
	glm::ivec2 getSizeHint() override;

	bool isDpiDependent() const override;

    AImage rasterize(glm::ivec2 imageSize) override;

};
