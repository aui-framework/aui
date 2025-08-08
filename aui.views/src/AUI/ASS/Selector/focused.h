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

//
// Created by alex2772 on 1/3/21.
//

#pragma once


namespace ass {
    template<typename Base>
    struct focused: Base {// ignore ass_selectors it's used in compound by class_of or t
        template<typename... Args>
        focused(Args&&... args):
                Base(std::forward<Args>(args)...)
        {

        }

        bool isStateApplicable(AView* view) override {
            return Base::isStateApplicable(view) && view->hasFocus();
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            Base::setupConnections(view, helper);
            view->focusState.clearAllOutgoingConnectionsWith(helper.get());
            AObject::connect(view->focusState, AUI_SLOT(helper)::onInvalidateStateAss);
        }
    };
}