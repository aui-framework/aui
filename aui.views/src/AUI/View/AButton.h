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
#include <AUI/Util/Declarative/Containers.h>

/**
 * ---
 * title: Button
 * icon: material/button-pointer
 * ---
 *
 * @brief Button with text, which can be pushed to make some action.
 *
 * ![](imgs/views/AButton.png)
 *
 * @ingroup views_actions
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
 * user presses ++enter++, the button is pushed automatically.
 *
 * Button usually contains text only, but in practice any view can be put in it.
 *
 * Starting from AUI 8.0.0, AButton itself does not render text; instead, it's a styled container, which is populated
 * with any views by the user, i.e., ALabel. AButton used to have
 * Qt-like methods for customization like `setIcon`, but now a modern approach takes place, which allows extensive
 * options of customization.
 *
 * AButton supports both [retained and immediate modes](retained_immediate_ui.md).
 *
 * ## API surface
 *
 * <!-- aui:steal_documentation declarative::Button -->
 *
 * ## Button with a lambda handler
 *
 * This button executes the lambda upon click.
 *
 * <!-- aui:snippet examples/ui/button1/src/main.cpp AButton_example -->
 *
 * ![](imgs/Screenshot_20250715_091801.png)
 *
 * ## Button with a signal-slot handler
 *
 * This button executes the member function upon click.
 *
 * <!-- aui:snippet examples/ui/button2/src/main.cpp AButton_example -->
 *
 * ![](imgs/Screenshot_20250715_091801.png)
 *
 * ## Default button
 *
 * Button can be made default. In such case, it is colored to user's accent color, making it stand out. Also, when the
 * user presses ++enter++, the button is pushed automatically.
 *
 * <!-- aui:snippet examples/ui/button_default/src/main.cpp AButton_example -->
 *
 * ![](imgs/Screenshot_20250719_130434.png).
 *
 * ## Button with icon
 *
 * While buttons typically display text, they are actually flexible containers that can hold any view component. This
 * means you can place various UI elements inside a button, such as images, icons, custom layouts, or combinations of
 * different views.
 *
 * <!-- aui:snippet examples/ui/button_icon/src/main.cpp AButton_example -->
 *
 * ![](imgs/Screenshot_20250719_130034.png)
 *
 * ## Styling a button
 *
 * AButton is styled as follows:
 *
 * <!-- aui:snippet aui.views/src/AUI/ASS/AStylesheet.cpp AButton -->
 */
class API_AUI_VIEWS AButton : public AViewContainer {
public:
    AButton();

    /**
     * @brief Inflates a label with a text.
     * @details
     * Left for compatibility.
     *
     * This setter would override any of existing content within button.
     */
    explicit AButton(AString text) {
        setText(std::move(text));
    }

    /**
     * @brief Inflates a label with a text.
     * @details
     * Left for compatibility.
     *
     * This setter would override any of existing content within button.
     */
    void setText(AString text);

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

namespace declarative {

/// [declarative_example]

/**
 * <!-- aui:no_dedicated_page -->
 */
struct Button {
    /**
     * @brief Content of the button.
     * @details
     * Can be any view, i.e., `Label` to display text.
     */
    _<AView> content;

    /**
     * @brief Handler for button click event.
     * @details
     * Called when user activates the button.
     */
    contract::Slot<> onClick;

    /**
     * @brief Determines if the button is default.
     * @details
     * Default buttons are colored with user's accent color and respond to ++enter++ key.
     */
    bool isDefault = false;

    _<AButton> operator()() {
        auto button = _new<AButton>();
        onClick.bindTo(button->clicked);
        if (isDefault) {
            button->setDefault();
        }
        button->setContents(Centered { std::move(content) });
        return button;
    }
};
/// [declarative_example]
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
