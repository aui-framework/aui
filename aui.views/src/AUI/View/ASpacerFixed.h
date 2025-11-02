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

#include <AUI/View/AView.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/Declarative/Containers.h>

/**
 *
 * ---
 * title: Spacers
 * icon: fontawesome/solid/arrows-left-right-to-line
 * ---
 *
 * @brief A transparent view with specific size used to create a blank space between 2 views.
 *
 * ![](imgs/Screenshot_20251102_081439.png)
 *
 * @ingroup views_arrangement
 * @details
 * Spacers are transparent views that acquire specified space.
 *
 * The spacer behaves differently based on its parent container's layout:
 *
 * - In Horizontal layouts: Consumes horizontal space (width).
 * - In Vertical layouts: Consumes vertical space (height).
 *
 * You can use [AUI Devtools](devtools.md) to reveal spacers in a layout.
 *
 * ## API surface
 *
 * <!-- aui:steal_documentation declarative::SpacerFixed -->
 *
 * <!-- aui:steal_documentation declarative::SpacerExpanding -->
 *
 * ## Fixed spacer
 *
 * `declarative::SpacerFixed` creates a fixed-size gap with a specified amount of space. Unlike [ass::LayoutSpacing],
 * which adds uniform gaps between all container's views, `SpacerFixed` creates a single customizable gap at a specific
 * location.
 *
 * <!-- aui:snippet examples/ui/button_icon/src/main.cpp AButton_example -->
 *
 * ![](imgs/Screenshot_20250719_130034.png)
 *
 * ## Expanding spacer
 *
 * `declarative::SpacerExpanding` is simply an [expanding](layout-managers.md#EXPANDING) view. It can be used to "push"
 * subsequent views within a container to the end of that container.
 *
 * In most cases, the container must have [ass::MinSize] or [expanding](layout-managers.md#EXPANDING) specified, as
 * shown in the example below.
 *
 * <!-- aui:snippet examples/ui/spacer_expanding1/src/main.cpp ASpacerExpanding_example -->
 *
 * <figure markdown="span">
 * ![](imgs/Screenshot_20251101_210248.png){ width="500" }
 * <figcaption>Figure 1. SpacerExpanding in vertical container.</figcaption>
 *
 * </figure>
 *
 * ---
 *
 * <!-- aui:snippet examples/ui/spacer_expanding2/src/main.cpp ASpacerExpanding_example -->
 *
 * <figure markdown="span">
 * ![](imgs/Screenshot_20251101_210531.png){ width="500" }
 * <figcaption>Figure 2. SpacerExpanding in horizontal container.</figcaption>
 * </figure>
 */
class API_AUI_VIEWS ASpacerFixed : public AView {
public:
    explicit ASpacerFixed(AMetric space) : mSpace(space) {}
    bool consumesClick(const glm::ivec2& pos) override;
    int getContentMinimumWidth() override;
    int getContentMinimumHeight() override;
    void render(ARenderContext ctx) override;
    void setSpace(AMetric space) { mSpace = space; markMinContentSizeInvalid(); }

private:
    AMetric mSpace;

    ALayoutDirection getParentLayoutDirection() const;
};

namespace declarative {

/**
 * <!-- aui:no_dedicated_page -->
 */
struct API_AUI_VIEWS SpacerFixed {
    /**
     * @brief Fixed space to acquire.
     */
    contract::In<AMetric> space;

    _<AView> operator()();
};
}   // namespace declarative