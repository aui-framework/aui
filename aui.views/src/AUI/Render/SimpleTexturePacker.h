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

#include "TexturePacker.h"
#include "AUI/Image/AImage.h"

namespace Util {
    class API_AUI_VIEWS SimpleTexturePacker : public ::Util::TexturePacker<AImage> {
    private:
        AOptional<AImage> mImage;
    public:
        SimpleTexturePacker();
        ~SimpleTexturePacker();
        glm::vec4 insert(AImage& data) override;
        SimpleTexturePacker(const SimpleTexturePacker&) = delete;

        AOptional<AImage>& getImage() {
            return mImage;
        }
    protected:
        void onResize(AImage& data, dim side) override;
        void onInsert(AImage& data, const dim& x, const dim& y) override;
    };
}



