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
#include "AUI/Layout/AStackedLayout.h"
#include <AUI/Util/Declarative/Containers.h>

/**
 * @brief Button with text, which can be pushed to make some action.
 * @ingroup useful_views
 * @details
 * Button is a fundamental view which can be pushed to make some action.
 *
 * @image html docs/imgs/AButton.gif "The button view being pressed."
 *
 * Starting from AUI 8.0.0, AButton itself does not render text; instead, it's a styled container, which is populated
 * with any views by the user, i.e., ALabel. `Button { .content = "Text" }` does exactly this. AButton used to have
 * Qt-like methods for customization like `setIcon`, but now a modern approach takes place, which allows extensive
 * options of customization.
 *
 * Button is styled with background, box shadow, and a border that highlights on hover. When pushed, the shadow
 * disappears, making an illusion of pressing.
 *
 * # Button with a lambda handler
 *
 * This button executes the lambda upon click.
 *
 * @snippet examples/ui/button1/src/main.cpp AButton example
 *
 * @image html docs/imgs/Screenshot_20250715_091801.png "Button with a lambda."
 *
 * # Button with a signal-slot handler
 *
 * This button executes the member function upon click.
 *
 * @snippet examples/ui/button2/src/main.cpp AButton example
 *
 * @image html docs/imgs/Screenshot_20250715_091801.png "Button with a signal-slot."
 *
 * # Default button
 *
 * Button can be made default. In such case, it is colored to user's accent color, making it stand out. Also, when the
 * user presses `Enter`, the button is pushed automatically.
 *
 * @snippet examples/ui/button_default/src/main.cpp AButton example
 *
 * @image html docs/imgs/Screenshot_20250719_130434.png A default button.
 *
 * # Button with icon
 *
 * Button usually contains text only, but in practice any view can be put in it.
 *
 * @snippet examples/ui/button_icon/src/main.cpp AButton example
 *
 * @image html docs/imgs/Screenshot_20250719_130034.png Button with an icon.
 *
 * # Styling a button
 *
 * AButton is styled as follows:
 *
 * @snippet aui.views/src/AUI/ASS/AStylesheet.cpp AButton
 */
class API_AUI_VIEWS AButton : public AViewContainer {
public:
    AButton();

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
/**
 * @declarativeformof{AButton}
 */
struct Button {
    std::variant<contract::In<AString>, _<AView>> content;
    contract::Slot<> onClick;
    bool isDefault = false;

    _<AButton> operator()() {
        auto button = _new<AButton>();
        onClick.bindTo(button->clicked);
        if (isDefault) {
            button->setDefault();
        }
        if (auto* s = std::get_if<contract::In<AString>>(&content)) {
            // if .content = "some text", compose a label for it.
            auto label = _new<ALabel>();
            s->bindTo(label->text());
            button->setContents(Centered { std::move(label) });
            return button;
        }
        button->setContents(Centered { std::move(std::get<_<AView>>(content)) });
        return button;
    }
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
