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
#include <AUI/Util/Declarative/Containers.h>

class API_AUI_VIEWS ASpacerExpanding : public AView {
public:
    ASpacerExpanding(int w = 4, int h = 4) { setExpanding({ w, h }); }
    virtual ~ASpacerExpanding() = default;

    bool consumesClick(const glm::ivec2& pos) override;
};

namespace declarative {
/**
 * <!-- aui:no_dedicated_page -->
 */
struct API_AUI_VIEWS SpacerExpanding {
    /**
     * @brief [Expanding](layout-managers.md#EXPANDING) to acquire.
     */
    contract::In<int> expanding = 12;
    _<AView> operator()();
};
}