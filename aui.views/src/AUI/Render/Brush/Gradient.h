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

#include "AUI/Image/AImage.h"
#include "AUI/Render/ABrush.h"

namespace aui::render::brush::gradient {
    struct Helper {
    public:
        ASmallVector<glm::u8vec4, 8> colors;
        glm::mat3 matrix;

        Helper(const ALinearGradientBrush& brush) {
            for (const auto& c : brush.colors) {
                colors.push_back(glm::uvec4(c.color * 255.f));
            }
            //float actualEdgeUvPosition = (0.5f - 0.05f /* bias to make ideal color edge */) / float(colors.size());
            float actualEdgeUvPosition = 0.f; // temporary

            float rotationRadians = brush.rotation.radians();

            float s = glm::sin(rotationRadians);
            float c = -glm::cos(rotationRadians);
            float bias = glm::max(-s, -c, 0.f);

            auto probe = [&](glm::vec2 uv) {
                return uv.x * s + uv.y * c + bias;
            };
            auto adjust = [&](glm::vec2 uv) {
                float p = probe(uv);
                if (p > 1.f) {
                    s /= p;
                    c /= p;
                    bias /= p;
                }
            };

            if (auto p = probe({1, 1}); p < 0.f) {
                bias += -p;
            }

            adjust({0, 0});
            adjust({1, 0});
            adjust({0, 1});
            adjust({1, 1});

            matrix = {
                /* uv.x */ s * (1.f - actualEdgeUvPosition * 2.f), 0.f, 0.f,
                /* uv.y */ c * (1.f - actualEdgeUvPosition * 2.f), 0.f, 0.f,
                /* 1    */ bias * (1.f - actualEdgeUvPosition * 2.f) + actualEdgeUvPosition, 0.f, 0.f,
            };
        }

        [[nodiscard]]
        AImageView gradientMap() const noexcept {
            return AImageView({(const char*)colors.data(), colors.sizeInBytes()}, {colors.size(), 1}, APixelFormat::RGBA_BYTE);
        }
    };
}