// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
// Created by alex2 on 23.10.2020.
//

#pragma once


#include <AUI/Image/IDrawable.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Image/AImage.h>
#include <AUI/Render/ARender.h>

class AImageDrawable: public IDrawable {
private:
    glm::ivec2 mSize;
    std::variant<ARender::Texture, _<AImage>> mStorage;

public:
    explicit AImageDrawable(_<AImage> image);
    virtual ~AImageDrawable();

    void draw(const Params& params) override;

    AImage rasterize(glm::ivec2 imageSize) override;

    glm::ivec2 getSizeHint() override;
};


