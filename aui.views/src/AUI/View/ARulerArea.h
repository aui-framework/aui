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

#include <AUI/View/AViewContainer.h>
#include "ARulerView.h"

/**
 * @brief Photoshop-like ruler area for measuring display units.
 *
 * ![](imgs/views/ARulerArea.png)
 *
 * @ingroup views_containment
 * @details
 * Consists of vertical and horizontal [rulers](arulerview.md).
 */
class API_AUI_VIEWS ARulerArea: public AViewContainerBase, public IFontView {
private:
    _<AView> mWrappedView;
    _<ARulerView> mHorizontalRuler;
    _<ARulerView> mVerticalRuler;
    glm::ivec2 mMousePos;

    void setWrappedViewPosition(const glm::ivec2& pos);

    glm::ivec2 getTargetPosition() const;

protected:
    void updatePosition();

public:
    ARulerArea(const _<AView>& wrappedView);
    void setSize(glm::ivec2 size) override;
    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;

    void invalidateFont() override;

    void render(ARenderContext ctx) override;
};

