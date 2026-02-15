// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include "AUI/Common/AVector.h"

#include <functional>
#include <initializer_list>
#include "AUI/Common/ASmallVector.h"
#include <utility>

class AView;

namespace ass {
/**
 * @brief A lightweight container for view modifiers.
 *
 * A modifier is a callable that receives a reference to an {@link AView} and applies
 * some change to it (e.g. set a background color, change a size, etc.).
 *
 * Modifiers are immutable – every operation returns a new instance.  They can be
 * combined using the `|` operator.  When a modifier is applied to a view, the
 * stored callables are executed in the order they were added.  Because the
 * last modifier is executed last, it naturally implements a *last‑wins*
 * strategy for properties that are set by multiple modifiers.
 */
class Modifier {
public:
    using Element = std::function<void(AView&)>;

    Modifier() = default;
    Modifier(const Modifier& other) = default;
    Modifier(Modifier&& other) noexcept = default;
    Modifier& operator=(const Modifier& other) = default;
    Modifier& operator=(Modifier&& other) noexcept = default;

    [[nodiscard]] const auto& elements() const { return mElements; }

private:
    friend Modifier operator|(Modifier lhs, aui::convertible_to<Modifier::Element> auto && rhs);
    ASmallVector<Element, 16> mElements;
};

/**
 * @brief Combines two modifiers.
 *
 * Defined as a free function so that it can be found by ADL when the left
 * operand is a `Modifier` and the right operand is also a `Modifier`.
 */
inline Modifier operator|(Modifier lhs, aui::convertible_to<Modifier::Element> auto && rhs) {
    lhs.mElements << rhs;
    return lhs;
}
}
