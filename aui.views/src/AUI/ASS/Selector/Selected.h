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

#include <AUI/Util/ADataBinding.h>
#include "AAssSelector.h"

namespace ass {
    class ISelectable {
    friend struct Selected;
    protected:
        virtual bool selectableIsSelectedImpl() = 0;
    };

    struct Selected: IAssSubSelector {
    private:
        _unique<IAssSubSelector> mWrapped;

    public:
        template<typename T>
        Selected(T value): mWrapped(new T(std::move(value))) {}


        bool isPossiblyApplicable(AView* view) override {
            return mWrapped->isPossiblyApplicable(view) && dynamic_cast<ISelectable*>(view);
        }

        bool isStateApplicable(AView* view) override {
            if (!mWrapped->isStateApplicable(view))
                return false;

            if (auto c = dynamic_cast<ISelectable*>(view)) {
                return c->selectableIsSelectedImpl();
            }
            return false;
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            IAssSubSelector::setupConnections(view, helper);
            mWrapped->setupConnections(view, helper);

            view->customCssPropertyChanged.clearAllConnectionsWith(helper.get());
            AObject::connect(view->customCssPropertyChanged, slot(helper)::onInvalidateStateAss);
        }
    };
}