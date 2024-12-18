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

//
// Created by alex2 on 02.01.2021.
//

#pragma once


namespace ass {
    template<typename Base>
    struct hovered: Base {
        template<typename... Args>
        hovered(Args&&... args):
            Base(std::forward<Args>(args)...)
        {

        }

        bool isStateApplicable(AView* view) override {
            return Base::isStateApplicable(view) && view->isEnabled() && view->isMouseHover();
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            Base::setupConnections(view, helper);
            view->hoveredState.clearAllConnectionsWith(helper.get());
            AObject::connect(view->hoveredState, slot(helper)::onInvalidateStateAss);
        }
    };
}