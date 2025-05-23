﻿/*
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
#include "AUI/Common/AString.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "ALabel.h"
#include "AViewContainer.h"
#include "AUI/ASS/Selector/AAssSelector.h"

/**
 * @brief Button with text, which can be pushed to make some action.
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/AButton.gif">
 * @ingroup useful_views
 * @details
 * See @ref declarative::Button for a declarative form and examples.
 */
class API_AUI_VIEWS AButton : public AAbstractLabel {
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
class AButtonEx : public AViewContainer {
public:
    AButtonEx() {
        addAssName(".btn");
    }

    ~AButtonEx() override = default;
};

namespace declarative {
/**
 * @declarativeformof{AButton}
 */
struct Button : aui::ui_building::view_container_layout<AHorizontalLayout, AButtonEx> {
    /**
     * @brief Basic label initializer.
     * @details
     * @code{cpp}
     * Button { "Action label" }.connect(&AView::clicked, this, [] {
     *   // action
     * }),
     * @endcode
     */
    Button(AString text) : layouted_container_factory<AHorizontalLayout, AButtonEx>({ Label { std::move(text) } }) {}

    /**
     * @brief Basic label initializer.
     * @details
     * @code{cpp}
     * Button { "Action label" }.connect(&AView::clicked, this, [] {
     *   // action
     * }),
     * @endcode
     */
    Button(const char* text) : layouted_container_factory<AHorizontalLayout, AButtonEx>({ Label { text } }) {}

    /**
     * @brief An explicit form of AButton where you can put any views in it, i.e., icons.
     * @details
     * @code{cpp}
     * Button {
     *   Icon { ":img/cart.svg" },
     *   Label { "Cart" },
     * }.connect(&AView::clicked, this, [] {
     *   // action
     * }),
     * @endcode
     */
    template <typename... Views>
    Button(Views&&... views)
        : layouted_container_factory<AHorizontalLayout, AButtonEx>(std::forward<Views>(views)...) {}
};
}   // namespace declarative

namespace ass::button {
struct Default : IAssSubSelector {
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
                c->defaultState.clearAllOutgoingConnectionsWith(helper.get());
                AObject::connect(c->defaultState, slot(helper)::onInvalidateStateAss);
            }
        }
    };
}