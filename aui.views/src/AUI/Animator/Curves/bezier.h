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
