// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by Alex2772 on 2/8/2022.
//

#include "SvgImageFactory.h"
#include "AUI/Common/AByteBuffer.h"

#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg.h>
#include <nanosvgrast.h>


SvgImageFactory::SvgImageFactory(AByteBufferView buf) {
    AByteBuffer copy;
    copy << buf << '\0';
    mNsvg = nsvgParse(copy.data(), "px", 96.f);
}

SvgImageFactory::~SvgImageFactory() {
    nsvgDelete(static_cast<NSVGimage*>(mNsvg));
}

AImage SvgImageFactory::provideImage(const glm::ivec2& size) {
    AImage image(size.x, size.y, AImageFormat::RGBA | AImageFormat::BYTE);
    image.allocate();
    auto rasterizer = nsvgCreateRasterizer();
    assert(rasterizer);
    auto svg = static_cast<NSVGimage*>(mNsvg);
    nsvgRasterize(rasterizer, svg, 0, 0, glm::min(size.x / svg->width, size.y / svg->height),
                  reinterpret_cast<unsigned char*>(image.getData().data()), size.x, size.y, size.x * 4);
    nsvgDeleteRasterizer(rasterizer);
    return image;
}

glm::ivec2 SvgImageFactory::getSizeHint() {
    auto svg = static_cast<NSVGimage*>(mNsvg);
    return {svg->width, svg->height};
}
