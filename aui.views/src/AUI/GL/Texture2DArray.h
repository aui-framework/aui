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

#include "AUI/Common/SharedPtr.h"
#include "AUI/Image/AImage.h"
#include "Texture.h"
#include "GLEnums.h"

namespace gl {
	class API_AUI_VIEWS Texture2DArray: public Texture<gl::TEXTURE_2D_ARRAY> {
	public:
        void tex3D(const AVector<AImageView>& images);
        virtual ~Texture2DArray() = default;

      private:
          glm::u32vec3 mSize = {0, 0, 0};
	};
}
