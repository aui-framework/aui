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

#include <glm/glm.hpp>
#include <AUI/Common/AColor.h>

class ITexture;

namespace aui {
    struct API_AUI_VIEWS CharacterData {
        glm::vec4 uv;
        _<ITexture> texture;
    };

    struct GlyphInstance {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec2 u1;
        glm::vec2 u2;
        AColor color;
        _<ITexture> texture;
    };
}
