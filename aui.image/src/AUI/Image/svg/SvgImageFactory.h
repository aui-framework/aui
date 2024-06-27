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


#include "AUI/Image/IImageFactory.h"
#include "AUI/Common/AByteBufferView.h"

namespace lunasvg {
    class Document;
}

class API_AUI_IMAGE SvgImageFactory: public IImageFactory {
public:
    SvgImageFactory(AByteBufferView buf);
    ~SvgImageFactory();
    AImage provideImage(const glm::ivec2& size) override;

    glm::ivec2 getSizeHint() override;

private:
    std::unique_ptr<lunasvg::Document> mImpl;
};


