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
#include "AWordWrappingEngine.h"

class API_AUI_VIEWS AViewEntry: public AWordWrappingEngine::Entry {
private:
    _<AView> mView;

public:
    explicit AViewEntry(const _<AView>& view) : mView(view) {}
    AViewEntry() = default;

    glm::ivec2 getSize() override;
    void setPosition(const glm::ivec2& position) override;
    AFloat getFloat() const override;

    ~AViewEntry() override;
};


