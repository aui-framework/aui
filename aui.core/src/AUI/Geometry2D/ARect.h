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
#include "ALineSegment.h"
#include <array>
#include <range/v3/algorithm/any_of.hpp>

/**
 * @brief Axis aligned 2D rectangle.
 * @ingroup core
 */
template<typename T>
struct ARect {
    APoint2D<T> p1, p2;

    static ARect fromTopLeftPositionAndSize(APoint2D<T> position, APoint2D<T> size) {
        return { .p1 = position, .p2 = position + size };
    }

    static ARect fromCenterPositionAndSize(APoint2D<T> position, APoint2D<T> size) {
        return { .p1 = position - size / static_cast<T>(2), .p2 = position + size / static_cast<T>(2) };
    }

    [[nodiscard]]
    bool operator==(const ARect&) const noexcept = default;

    [[nodiscard]]
    APoint2D<T> leftTop() const noexcept {
        return min();
    }

    [[nodiscard]]
    APoint2D<T> rightBottom() const noexcept {
        return max();
    }

    [[nodiscard]]
    APoint2D<T> rightTop() const noexcept {
        return { glm::max(p1.x, p2.x), glm::min(p1.y, p2.y) };
    }

    [[nodiscard]]
    APoint2D<T> leftBottom() const noexcept {
        return { glm::min(p1.x, p2.x), glm::max(p1.y, p2.y) };
    }

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
        return (p1 + p2) / static_cast<T>(2);
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

    [[nodiscard]]
    ALineSegment<T> topLineSegment() const noexcept {
        return { .p1 = APoint2D<T>(p1.x, p1.y), .p2 = APoint2D<T>(p2.x, p1.y) };
    }

    [[nodiscard]]
    ALineSegment<T> leftLineSegment() const noexcept {
        return { .p1 = APoint2D<T>(p1.x, p1.y), .p2 = APoint2D<T>(p1.x, p2.y) };
    }

    [[nodiscard]]
    ALineSegment<T> rightLineSegment() const noexcept {
        return { .p1 = APoint2D<T>(p2.x, p1.y), .p2 = APoint2D<T>(p2.x, p2.y) };
    }


    [[nodiscard]]
    ALineSegment<T> bottomLineSegment() const noexcept {
        return { .p1 = APoint2D<T>(p1.x, p2.y), .p2 = APoint2D<T>(p2.x, p2.y) };
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

        {
            auto verticalLines = [](const ARect& r) {
                return std::array<ALineSegment<T>, 2>{ r.leftLineSegment(), r.rightLineSegment() };
            };
            auto horizontalLines = [](const ARect& r) {
                return std::array<ALineSegment<T>, 2>{ r.topLineSegment(), r.bottomLineSegment() };
            };

            //        this
            //      +------+
            //      |      |
            //+-----+------+-----+
            //|     |      |     |
            //|     |      |     | other
            //|     |      |     |
            //+-----+------+-----+
            //      |      |
            //      +------+
            if (ranges::any_of(verticalLines(*this), [&](const ALineSegment<T>& l) {
                return ranges::any_of(horizontalLines(other), [&](const ALineSegment<T>& r) {
                    return l.isIntersects(r);
                });
            })) {
                return true;
            }

            //       other
            //      +------+
            //      |      |
            //+-----+------+-----+
            //|     |      |     |
            //|     |      |     | this
            //|     |      |     |
            //+-----+------+-----+
            //      |      |
            //      +------+
            if (ranges::any_of(horizontalLines(*this), [&](const ALineSegment<T>& l) {
                return ranges::any_of(verticalLines(other), [&](const ALineSegment<T>& r) {
                    return l.isIntersects(r);
                });
            })) {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]]
    APoint2D<T> size() const noexcept {
        return glm::abs(p2 - p1);
    }

    ARect& translate(glm::ivec2 by) noexcept {
        p1 += by;
        p2 += by;
        return *this;
    }
};
