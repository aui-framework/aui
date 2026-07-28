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
// Created by alex2 on 5/23/2021.
//


#pragma once

#include "AAbstractTypeable.h"
#include <AUI/Platform/AWindow.h>

/**
 * @brief Basic implementation of type shortcuts and selection for editable text fields.
 * @details Used as base in ATextArea and ATextField, both of them using own way of text handling and rendering.
 */
template<aui::derived_from<AView> Super>
class AAbstractTypeableView
        : public Super,
          public AAbstractTypeable,
          public std::conditional_t<aui::derived_from<Super, IFontView>, std::monostate, IFontView> /* implement
                                                                                                       IFontView if
                                                                                                       Super doesn't */{
public:
    AAbstractTypeableView() {
        AObject::connect(mBlinkTimer->fired, this, [&]() {
            if (this->hasFocus() && mCursorBlinkCount < 60) {
                mCursorBlinkVisible = !mCursorBlinkVisible;
                mCursorBlinkCount += 1;
                this->redraw();
            }
        });
    }


    /**
     * @brief Text property.
     */
    auto text() const {
        return APropertyDef {
            this,
            &AAbstractTypeable::getText,
            &AAbstractTypeable::setText,
            textChanging,
        };
    }

    ~AAbstractTypeableView() override = default;

    void onKeyDown(AInput::Key key) override {
        Super::onKeyDown(key);
        if (key == AInput::ESCAPE) {
            AWindow::current()->setFocusedView(nullptr);
        }
        AAbstractTypeable::handleKey(key);
        if (key == AInput::Key::RETURN && !AInput::isKeyDown(AInput::LSHIFT) && !AInput::isKeyDown(AInput::RSHIFT)) {
            emit this->actionButtonPressed;
        }
    }

    void onKeyRepeat(AInput::Key key) override {
        Super::onKeyRepeat(key);
        AAbstractTypeable::handleKey(key);
    }

    void onFocusLost() override {
        Super::onFocusLost();
        if (mTextChangedFlag) {
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

    bool wantsTouchscreenKeyboard() override {
        return true;
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
        this->invalidateAllStylesFont();
        Super::invalidateAllStyles();
    }

    bool isLButtonPressed() override {
        return this->isPressed();
    }

    void drawCursor(IRenderer& renderer, glm::ivec2 position) {
        if (!this->hasFocus()) {
            return;
        }
        drawCursorImpl(renderer, position, this->getFontStyle().size + this->getFontStyle().getDescenderHeight());
    }

protected:
    AMenuModel composeContextMenu() override {
        return composeContextMenuImpl();
    }

    void commitStyle() override {
        Super::commitStyle();
        this->commitStyleFont();
    }


    void cursorSelectableRedraw() override {
        this->redraw();
    }

    void onSelectionChanged() override {
        onCursorIndexChanged();
        if (selectionChanged) emit selectionChanged(selection());
    }

private:
    void emitTextChanged(const AString& text) override {
        emit textChanged(text);
    }

    void emitTextChanging(const AString& text) override {
        emit textChanging(text);
    }

    void emitActionButtonPressed() override {
        emit actionButtonPressed;
    }

    void typeableInvalidateFont() override {
        this->invalidateFont();
    }
};