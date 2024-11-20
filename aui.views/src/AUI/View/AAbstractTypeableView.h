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
// Created by alex2 on 5/23/2021.
//


#pragma once

#include "AAbstractTypeable.h"

/**
 * @brief Basic implementation of type shortcuts and selection for editable text fields.
 * @details Used as base in ATextArea and ATextField, both of them using own way of text handling and rendering.
 */
template<aui::derived_from<AView> Super>
class AAbstractTypeableView: public Super, public AAbstractTypeable {
public:
    AAbstractTypeableView() {
        AObject::connect(mBlinkTimer->fired, this, [&]()
        {
            if (this->hasFocus() && mCursorBlinkCount < 60) {
                mCursorBlinkVisible = !mCursorBlinkVisible;
                mCursorBlinkCount += 1;
                this->redraw();
            }
        });
    }
    ~AAbstractTypeableView() override = default;

    void onKeyDown(AInput::Key key) override {
        Super::onKeyDown(key);
        AAbstractTypeable::handleKey(key);
    }

    void onKeyRepeat(AInput::Key key) override {
        Super::onKeyRepeat(key);
        AAbstractTypeable::handleKey(key);
    }

    void onFocusLost() override {
        Super::onFocusLost();
        if (mTextChangedFlag)
        {
            mTextChangedFlag = false;
            if (textChanged) {
                emit textChanged(text());
            }
        }
    }

    void onPointerPressed(const APointerPressedEvent& event) override {
        Super::onPointerPressed(event);
        ACursorSelectable::handleMousePressed(event);
        updateCursorBlinking();
    }

    void onPointerDoubleClicked(const APointerPressedEvent& event) override {
        Super::onPointerDoubleClicked(event);
        ACursorSelectable::handleMouseDoubleClicked(event);
        updateCursorBlinking();
    }

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override {
        Super::onPointerMove(pos, event);
        ACursorSelectable::handleMouseMove(pos);
    }

    void onPointerReleased(const APointerReleasedEvent& event) override {
        Super::onPointerReleased(event);
        if (!event.triggerClick) return;

        if (event.pointerIndex != APointerIndex::button(AInput::RBUTTON)) {
            ACursorSelectable::handleMouseReleased(event);
        }
    }

    bool handlesNonMouseNavigation() override {
        return true;
    }

    void onFocusAcquired() override {
        Super::onFocusAcquired();
        updateCursorBlinking();
    }

    void invalidateAllStyles() override {
        // order is intentional
        invalidateAllStylesFont();
        Super::invalidateAllStyles();
    }

    bool isLButtonPressed() override {
        return this->isPressed();
    }

protected:
    AMenuModel composeContextMenu() override {
        return composeContextMenuImpl();
    }

    void commitStyle() override {
        Super::commitStyle();
        commitStyleFont();
    }

    int getVerticalAlignmentOffset() noexcept {
        return (glm::max)(0, int(glm::ceil((Super::getContentHeight() - getFontStyle().size) / 2.0)));
    }

    void cursorSelectableRedraw() override {
        this->redraw();
    }

    void emitTextChanged(const AString& text) override {
        emit textChanged(text);
    }

    void emitTextChanging(const AString& text) override {
        emit textChanging(text);
    }

    void onSelectionChanged() override {
        if (selectionChanged) emit selectionChanged(selection());
    }

    glm::ivec2 getMouseSelectionPadding() override {
        return {this->getPadding().left, this->getPadding().top + getVerticalAlignmentOffset() };
    }

    void updateCursorPos() override {
        auto absoluteCursorPos = -mHorizontalScroll + int(getFontStyle().getWidth(getDisplayText().substr(0, mCursorIndex)));

        const int SCROLL_ADVANCEMENT = this->getContentWidth() * 4 / 10;

        if (absoluteCursorPos < 0)
        {
            mHorizontalScroll += absoluteCursorPos - SCROLL_ADVANCEMENT;
        }
        else if (absoluteCursorPos >= this->getContentWidth())
        {
            mHorizontalScroll += absoluteCursorPos - this->getContentWidth() + SCROLL_ADVANCEMENT;
        }
        mHorizontalScroll = glm::clamp(mHorizontalScroll, 0, glm::max(int(getFontStyle().getWidth(getDisplayText())) - this->getContentWidth() + 1, 0));
    }
};