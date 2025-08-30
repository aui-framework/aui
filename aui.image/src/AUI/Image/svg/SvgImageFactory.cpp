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
// Created by Alex2772 on 2/8/2022.
//

#include "SvgImageFactory.h"
#include "AUI/Common/AByteBuffer.h"
#if defined(__has_include)
    #if __has_include(<lunasvg/lunasvg.h>)
        #include <lunasvg/lunasvg.h>
    #else
        #include <lunasvg.h>
    #endif
#else
    #include <lunasvg.h>
#endif


SvgImageFactory::SvgImageFactory(AByteBufferView buf) {
    mImpl = lunasvg::Document::loadFromData(buf.data(), buf.size());
    if (!mImpl) {
        throw AException("could not parse svg");
    }
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
