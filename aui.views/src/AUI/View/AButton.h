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
#include "AView.h"
#include "AUI/Common/AString.h"
#include "AUI/Render/ARender.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "ALabel.h"
#include "AViewContainer.h"
#include "AUI/ASS/Selector/AAssSelector.h"

/**
 * @brief Button with text, which can be pushed to make some action.
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/AButton.gif">
 * @ingroup useful_views
 */
class API_AUI_VIEWS AButton: public AAbstractLabel
{	
public:
	AButton();
	explicit AButton(AString text) noexcept: AAbstractLabel(std::move(text)) {}
	virtual ~AButton() = default;

    [[nodiscard]]
    bool isDefault() const noexcept {
        return mDefault;
    }

	void setDefault(bool isDefault = true);

    bool consumesClick(const glm::ivec2& pos) override;

signals:
	emits<bool> defaultState;
	emits<> becameDefault;
	emits<> noLongerDefault;

private:
	AFieldSignalEmitter<bool> mDefault = AFieldSignalEmitter<bool>(defaultState, becameDefault, noLongerDefault);
};

/**
 * @brief Unlike AButton, AButtonEx is a container which looks like a button.
 */
class AButtonEx: public AViewContainer {
public:
    AButtonEx() {
        addAssName(".btn");
    }
};

namespace declarative {
    struct Button: aui::ui_building::layouted_container_factory<AHorizontalLayout, AButtonEx> {
        using aui::ui_building::layouted_container_factory<AHorizontalLayout, AButtonEx>::layouted_container_factory;
        Button(AString text): layouted_container_factory<AHorizontalLayout, AButtonEx>({Label { std::move(text) }}) {}
        Button(const char* text): layouted_container_factory<AHorizontalLayout, AButtonEx>({Label { text }}) {}
    };
}

namespace ass::button {
    struct Default: IAssSubSelector {
    private:
        _unique<IAssSubSelector> mWrapped;

    public:
        template<typename T>
        Default(T value): mWrapped(new T(std::move(value))) {}


        bool isPossiblyApplicable(AView* view) override {
            return mWrapped->isPossiblyApplicable(view) && dynamic_cast<AButton*>(view);
        }

        bool isStateApplicable(AView* view) override {
            if (!mWrapped->isStateApplicable(view))
                return false;

            if (auto c = dynamic_cast<AButton*>(view)) {
                return c->isDefault();
            }
            return false;
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            IAssSubSelector::setupConnections(view, helper);
            mWrapped->setupConnections(view, helper);

            if (auto c = dynamic_cast<AButton*>(view)) {
                c->defaultState.clearAllConnectionsWith(helper.get());
                AObject::connect(c->defaultState, slot(helper)::onInvalidateStateAss);
            }
        }
    };
}