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


namespace aui::animation_curves {
    /**
     * Use online configurators like https://cubic-bezier.com/
     */
    class CubicBezier {
    private:
        glm::vec2 v1, v2;
    public:
        CubicBezier(glm::vec2 v1, glm::vec2 v2) : v1(v1), v2(v2) {}

        float operator()(float t) {
            float oneMinusX = 1 - t;
            float oneMinusX2 = oneMinusX * oneMinusX;
            float oneMinusX3 = oneMinusX2 * oneMinusX;
            float x2 = t * t;
            float x3 = x2 * t;

            float result = 3 * v1.x * oneMinusX2 * t + 3 * v2.x * oneMinusX * x2 + 2 * x3 +
                           3 * v1.y * oneMinusX2 * t + 3 * v2.y * oneMinusX * x2 - t;
            return result;
        }
    };

    class Standard: public CubicBezier {
    public:
        Standard(): CubicBezier({ 0.4f, 0.0f }, { 0.2f, 1.f }) {}
    };
}
