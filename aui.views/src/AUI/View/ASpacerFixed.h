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

#include <AUI/View/AView.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/Declarative.h>

/**
 * @brief Fixed-size view which is useful in UI building.
 * @ingroup useful_views
 * @details
 *
 * ABlankSpace is fixed size blank view which acquires specified space.
 */
class API_AUI_VIEWS ASpacerFixed: public AView {
public:
    ASpacerFixed(AMetric space): mSpace(space) {}

    bool consumesClick(const glm::ivec2& pos) override;

    int getContentMinimumWidth(ALayoutDirection layout) override;

    int getContentMinimumHeight(ALayoutDirection layout) override;

private:
    AMetric mSpace;
};


namespace declarative {
    using SpacerFixed = aui::ui_building::view<ASpacerFixed>;
}