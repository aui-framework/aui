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

#include "InspectorRenderers.h"

#include <AUI/ASS/ASS.h>
#include <AUI/Common/AColor.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AViewContainer.h>

using namespace ass;
using namespace declarative;

// ─── IInspectableValueBase default impl ──────────────────────────────────────

AArc<AView> IInspectableValueBase::renderAsView() const {
    return _new<ALabel>(renderAsString());
}

// ─── AInspectableValue<AString> ──────────────────────────────────────────────

AArc<AView> AInspectableValue<AString>::renderAsView() const {
    return _new<ALabel>(value);
}

// ─── AInspectableValue<AColor> ───────────────────────────────────────────────

AInspectableValue<AColor>::AInspectableValue(AColor v) : value(std::move(v)) {}

AString AInspectableValue<AColor>::renderAsString() const {
    return value.toString();
}

AArc<AView> AInspectableValue<AColor>::renderAsView() const {
    return Horizontal {
        Stacked {} AUI_OVERRIDE_STYLE {
            FixedSize { 12_dp, 12_dp },
            BackgroundSolid { value },
            Border { 1_px, 0x505050_rgb },
            BorderRadius { 2_dp },
        },
        _new<ALabel>(value.toString()),
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } };
}

// ─── aui::inspector free functions ───────────────────────────────────────────

_<AView> aui::inspector::render(const AInspectable& value) {
    return value.renderAsView();
}

_<AView> aui::inspector::render(const IInspectable::DebugInspectorInfo& info) {
    auto rows = Vertical {} AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp } };
    for (const auto& [name, value] : info) {
        rows->addView(Horizontal {
            _new<ALabel>(name + ":") AUI_OVERRIDE_STYLE { Opacity { 0.7f } },
            aui::inspector::render(value),
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } });
    }
    return rows;
}
