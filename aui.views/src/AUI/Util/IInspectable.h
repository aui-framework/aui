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

#include <utility>
#include "AUI/Common/AVector.h"
#include "AUI/Common/AString.h"
#include "AInspectable.h"

/**
 * @brief Interface for objects that can expose debug inspector information.
 * @ingroup views
 * @details
 * Analogue to Jetpack Compose's `debugInspectorInfo`. Objects implementing this interface expose a set of named
 * properties describing their state to debugging/inspection tooling (e.g. devtools, ViewPropertiesView).
 *
 * Properties are stored as an ordered sequence of name-value pairs, preserving insertion order (as Compose's
 * `properties` does). Each property value is an `AInspectable`, which carries the original typed value and knows
 * how to render itself both as a plain string (CLI / agentic AI profiler) and as a rich view (GUI devtools).
 *
 * To add inspector support for a custom type, either ensure `fmt::formatter<T>` is available (the default
 * `AInspectableValue<T>` will use it automatically), or specialise `AInspectableValue<T>` to override
 * `renderAsView()` / `renderAsString()`.
 */
class IInspectable {
public:
    /**
     * @brief A single named property exposed to the inspector.
     * @details `first` is the property name; `second` holds the typed value via `AInspectable`.
     */
    using Property = std::pair<AString, AInspectable>;

    /**
     * @brief Ordered sequence of named properties exposed to the inspector.
     * @details Insertion order is preserved.
     */
    using DebugInspectorInfo = AVector<Property>;

    virtual ~IInspectable() = default;

    /**
     * @brief Produces debug inspector information for this object.
     * @return ordered list of named properties describing this object's state.
     */
    virtual DebugInspectorInfo debugInspectorInfo() = 0;
};
