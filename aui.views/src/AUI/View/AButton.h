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
#include "AUI/Common/AString.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "ALabel.h"
#include "AViewContainer.h"
#include "AUI/ASS/Selector/AAssSelector.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/Util/UIBuildingHelpers.h"

/**
 * @brief Button with text, which can be pushed to make some action.
 *
 * ![](imgs/Views/AButton.png)
 *
 * @ingroup useful_views
 * @details
 * Button is a fundamental view which can be pushed to make some action.
 *
 * <figure markdown="span">
 * ![](imgs/AButton.gif)
 * <figcaption>The button view being pressed.</figcaption>
 * </figure>
 *
 * Button is styled with background, box shadow, and a border that highlights on hover. When pushed, the shadow
 * disappears, making an illusion of pressing.
 *
 * Button can be made default. In such case, it is colored to user's accent color, making it stand out. Also, when the
 * user presses `Enter`, the button is pushed automatically.
 *
 * Button usually contains text only, but in practice any view can be put in it.
 *
 * <!-- aui:include examples/ui/button/src/main.cpp -->
 *
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
struct Button : aui::ui_building::view_container_layout<AStackedLayout, AButtonEx> {
    /**
     * @brief Basic label initializer.
     * @details
     * ```cpp
     * Button { "Action label" }.connect(&AView::clicked, this, [] {
     *   // action
     * }),
     * ```
     */
    Button(AString text) : layouted_container_factory<AStackedLayout, AButtonEx>({ Label { std::move(text) } }) {}

    /**
     * @brief Basic label initializer.
     * @details
     * ```cpp
     * Button { "Action label" }.connect(&AView::clicked, this, [] {
     *   // action
     * }),
     * ```
     */
    Button(const char* text) : layouted_container_factory<AStackedLayout, AButtonEx>({ Label { text } }) {}

    /**
     * @brief An explicit form of AButton where you can put any views in it, i.e., icons.
     * @details
     * ```cpp
     * Button {
     *   Icon { ":img/cart.svg" },
     *   Label { "Cart" },
     * }.connect(&AView::clicked, this, [] {
     *   // action
     * }),
     * ```
     */
    template <typename... Views>
    Button(Views&&... views)
        : layouted_container_factory<AStackedLayout, AButtonEx>(Horizontal { std::forward<Views>(views)... } ) {}
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
                AObject::connect(c->defaultState, AUI_SLOT(helper)::onInvalidateStateAss);
            }
        }
    };
}