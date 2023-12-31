//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 2/8/2022.
//

#include "SvgImageFactory.h"
#include "AUI/Common/AByteBuffer.h"
#include <lunasvg/lunasvg.h>


SvgImageFactory::SvgImageFactory(AByteBufferView buf) {
    mImpl = lunasvg::Document::loadFromData(buf.data(), buf.size());
}

SvgImageFactory::~SvgImageFactory() {
}


AImage SvgImageFactory::provideImage(const glm::ivec2& size) {
    auto bitmap = mImpl->renderToBitmap(size.x, size.y);
    bitmap.convertToRGBA();
    return {AByteBufferView(reinterpret_cast<const char*>(bitmap.data()), bitmap.stride() * size.y),
                      glm::uvec2(size),
                      APixelFormat::RGBA_BYTE};
}

glm::ivec2 SvgImageFactory::getSizeHint() {
    return {mImpl->width(), mImpl->height()};
}
