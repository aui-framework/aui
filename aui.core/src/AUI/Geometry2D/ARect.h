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
#include <range/v3/algorithm/any_of.hpp>

/**
 * @brief 2D rectangle.
 * @ingroup core
 */
template<typename T>
struct ARect {
    APoint2D<T> p1, p2;

    static ARect fromTopLeftPositionAndSize(APoint2D<T> position, APoint2D<T> size) {
        return { .p1 = position, .p2 = position + size };
    }

    static ARect fromCenterPositionAndSize(APoint2D<T> position, APoint2D<T> size) {
        return { .p1 = position - size / 2, .p2 = position + size / 2 };
    }

    bool operator==(const ARect&) const noexcept = default;

    [[nodiscard]]
    bool isInside(APoint2D<T> other) const noexcept {
        return glm::all(glm::greaterThan(other, p1)) && glm::all(glm::lessThan(other, p2));
    }

    [[nodiscard]]
    bool isIntersects(APoint2D<T> other) const noexcept {
        return glm::all(glm::greaterThanEqual(other, p1)) && glm::all(glm::lessThanEqual(other, p2));
    }

    [[nodiscard]]
    std::array<APoint2D<T>, 4> vertices() const noexcept {
        return std::array<APoint2D<T>, 4>{
            p1,
            APoint2D<T>(p1.x, p2.y),
            APoint2D<T>(p2.x, p1.y),
            p2
        };
    }

    [[nodiscard]]
    APoint2D<T> center() const noexcept {
        return (p1 + p2) / 2;
    }

    [[nodiscard]]
    APoint2D<T> min() const noexcept {
        return glm::min(p1, p2);
    }

    [[nodiscard]]
    APoint2D<T> max() const noexcept {
        return glm::max(p1, p2);
    }

    [[nodiscard]]
    T area() const noexcept {
        return size().x * size().y;
    }

    /**
     * @return true if other intersects with this (even if intersection area =0!)
     */
    [[nodiscard]]
    bool isIntersects(ARect other) const noexcept {
        // equal rects intersects.
        if (*this == other) {
            return true;
        }

        // at least one of their point
        if (ranges::any_of(other.vertices(), [&](APoint2D<T> p) { return isIntersects(p); })) {
            return true;
        }

        // at least one of our point
        if (ranges::any_of(vertices(), [&](APoint2D<T> p) { return other.isIntersects(p); })) {
            return true;
        }

        //       other
        //      +------+
        //      |      |
        //+-----a------+-----+
        //|     |      |     |
        //|     |      |     | this
        //|     |      |     |
        //+-----+------+-----+
        //      |      |
        //      +------+
        {
            const auto a = APoint2D<T>(p2.x, other.p1.y);
            if (isIntersects(a) && other.isIntersects(a)) {
                return true;
            }
        }

        //       this
        //      +------+
        //      |      |
        //+-----a------+-----+
        //|     |      |     |
        //|     |      |     | other
        //|     |      |     |
        //+-----+------+-----+
        //      |      |
        //      +------+
        {
            const auto a = APoint2D<T>(p1.x, other.p2.y);
            if (isIntersects(a) && other.isIntersects(a)) {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]]
    APoint2D<T> size() const noexcept {
        return p2 - p1;
    }

    ARect& translate(glm::ivec2 by) noexcept {
        p1 += by;
        p2 += by;
        return *this;
    }
};
