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

#include <AUI/Common/SharedPtr.h>
#include "../Util/AInspectable.h"
#include "../Util/IInspectable.h"
#include <AUI/Views.h>

class AView;

/**
 * @brief Renders IInspectable::DebugInspectorInfo values into views for devtools.
 * @ingroup views
 * @details
 * Dispatches through `AInspectable::renderAsView()` — a virtual call on the typed `AInspectableValue<T>`.
 * The default implementation uses `fmt::formatter<T>` to produce a plain label; to customise rendering for
 * a specific type, specialise `AInspectableValue<T>` and override `renderAsView()` / `renderAsString()`.
 *
 * Built-in specialisations are provided for `AColor` (colour swatch + text), `AString` (direct label),
 * and `IInspectable::DebugInspectorInfo` (recursive row layout).
 */
namespace aui::inspector {

/**
 * @brief Produces a view representing the given inspectable value.
 * @details Delegates to `AInspectable::renderAsView()`.
 */
API_AUI_VIEWS _<AView> render(const AInspectable& value);

/**
 * @brief Produces a view representing the given DebugInspectorInfo (an ordered list of name-value rows).
 */
API_AUI_VIEWS _<AView> render(const IInspectable::DebugInspectorInfo& info);

}  // namespace aui::inspector
