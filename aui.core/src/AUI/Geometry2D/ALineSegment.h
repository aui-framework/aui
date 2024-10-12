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

#include "APoint2D.h"
#include <array>

/**
 * @brief 2D line segment.
 * @ingroup core
 */
template<typename T>
struct ALineSegment {
    APoint2D<T> p1, p2;

    [[nodiscard]]
    T length() const noexcept {
        return glm::distance(p1, p2);
    }

    [[nodiscard]]
    bool isIntersects(ALineSegment other) const noexcept {
        // suppose the two line segments run from p to p + r
        auto p = p1;
        auto r = p2 - p1;
        // and from q to q + s
        auto q = other.p1;
        auto s = other.p2 - other.p1;
        // then any point on the first line is representable as p + tr (for a scalar parameter t) and any point on the
        // second line as q + us (for a scalar parameter u) where both t and u >= 0 and <= 1.
        //
        // the two lines intersect if we can find t and u such that:
        //
        //    p + tr = q + us
        //
        // solve for t:
        // cross both sides by s
        //
        //    (p + tr) cross s = (q + us) cross s
        //
        // since s cross s = 0, this means
        //
        //    p cross s + tr cross s = q cross s
        //    tr cross s = (q - p) cross s
        //    t = (q - p) cross s / (r cross s)
        //
        // solve for u as well:
        //
        //    u = (q - p) cross r / (r cross s)

        auto cross = [](glm::vec<2, T> v, glm::vec<2, T> w) {
            return v.x * w.y - v.y * w.x;
        };

        // slightly modified to use lambda cross above:
        //     t = cross(q - p, s) / cross(r, s); t >= 0; t <= 1
        //     u = cross(q - p, r) / cross(r, s); u >= 0; u <= 1
        //
        // to support integer values properly, lets avoid division by multiplying both sides by cross(r, s)
        //
        // t = cross(q - p, s); t >= 0; t <= cross(r, s)
        // u = cross(q - p, r); u >= 0; u <= cross(r, s)
        auto rCrossS = cross(r, s);
        auto t = cross(q - p, s) * glm::sign(rCrossS);
        auto u = cross(q - p, r) * glm::sign(rCrossS);
        rCrossS = glm::abs(rCrossS);

        return t >= 0 && u >= 0 && t <= rCrossS && u <= rCrossS;
    }
};
