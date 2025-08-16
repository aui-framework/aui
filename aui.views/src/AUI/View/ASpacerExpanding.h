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
#include "AView.h"
#include <AUI/Util/Declarative.h>

/**
 * @brief Expanding view which is useful in UI building.
 * @ingroup useful_views
 * @details
 * AExpandingSpacer is an expanding blank view which tries acquire space as much as possible.
 *
 * See [declarative::SpacerExpanding] for a declarative form and examples.
 *
 * See expanding in [layout managers](layout-managers) for more info.
 *
 * @sa AView::setExpanding
 */
class API_AUI_VIEWS ASpacerExpanding : public AView {
public:
    ASpacerExpanding(int w = 4, int h = 4) { setExpanding({ w, h }); }
    virtual ~ASpacerExpanding() = default;

    bool consumesClick(const glm::ivec2& pos) override;
};

namespace declarative {
/**
 * @declarativeformof{ASpacerExpanding}
 */
struct SpacerExpanding: aui::ui_building::view<ASpacerExpanding> {
    SpacerExpanding() = default;
};
}