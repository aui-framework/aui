// AUI Framework - Declarative UI toolkit for modern C++20
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

#pragma once
#include <functional>
#include <AUI/Common/SharedPtrTypes.h>

#include "AUI/Common/ADeque.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Image/IImageFactory.h"
#include "AUI/Render/ARender.h"


class AVectorDrawable: public IDrawable
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

	void draw(const Params& params) override;
	glm::ivec2 getSizeHint() override;

	bool isDpiDependent() const override;

    AImage rasterize(glm::ivec2 imageSize) override;

};
