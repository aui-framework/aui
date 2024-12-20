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
// Created by alex2 on 09.12.2020.
//

#pragma once


#include <cstdint>
#include <AUI/Common/AVector.h>
#include <AUI/Common/AColor.h>
#include "Texture.h"

namespace gl {
    class API_AUI_VIEWS Texture1D: public Texture<TEXTURE_1D> {
    public:
        void tex1D(const AVector<AColor>& image);
        virtual ~Texture1D() = default;
    };
}