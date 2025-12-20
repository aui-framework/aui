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
#include <AUI/Util/AWordWrappingEngine.h>

class API_AUI_VIEWS AViewEntry final: public AWordWrappingEngine::Entry {
private:
    _<AView> mView;

public:
    explicit AViewEntry(_<AView> view) : mView(std::move(view)) {}
    AViewEntry() = default;

    glm::ivec2 getSize() override;
    void setPosition(glm::ivec2 position) override;
    AFloat getFloat() const override;

    ~AViewEntry() override;
};


