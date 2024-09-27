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
// Created by alex2 on 30.11.2020.
//

#include <AUI/Platform/AClipboard.h>
#include "ASelectableLabel.h"
#include <AUI/GL/gl.h>
#include <AUI/Render/RenderHints.h>
#include <glm/ext/matrix_transform.hpp>

ASelectableLabel::ASelectableLabel() {}

ASelectableLabel::ASelectableLabel(const AString& text) : ALabel(text) {}

glm::ivec2 ASelectableLabel::getMouseSelectionPadding() {
    return {mPadding.left, mPadding.top};
}

glm::ivec2 ASelectableLabel::getMouseSelectionScroll() {
    return {0, 0};
}

AFontStyle ASelectableLabel::getMouseSelectionFont() {
    return getFontStyleLabel();
}

AString ASelectableLabel::getDisplayText() {
    // TODO STUB
    return "";
    //return getTransformedText();
}

void ASelectableLabel::doRedraw() {
    redraw();
}

void ASelectableLabel::render(ARenderContext context) {
    AView::render(context);

    if (hasFocus()) {
        auto x =  mTextLeftOffset;
        if (getFontStyleLabel().align == ATextAlign::CENTER) {
            x -= mPrerendered->getWidth() / 2.f;
        }
        {
            RenderHints::PushMatrix m;

            ctx.render.setTransform(glm::translate(glm::mat4(1.f), {x, 0, 0}));
            drawSelectionPre();
        }
        doRenderText();

        {
            RenderHints::PushMatrix m;
            ctx.render.setTransform(glm::translate(glm::mat4(1.f), {x, 0, 0}));
            drawSelectionPost();
        }
    } else {
        doRenderText();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ASelectableLabel::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AView::onPointerMove(pos, event);
    pos.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == ATextAlign::CENTER) {
        pos.x += mPrerendered->getWidth() / 2.f;
    }
    handleMouseMove(pos);
}

void ASelectableLabel::onPointerPressed(const APointerPressedEvent& event) {
    clearSelection();
    AView::onPointerPressed(event);
    auto position = event.position;
    position.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == ATextAlign::CENTER) {
        position.x += mPrerendered->getWidth() / 2.f;
    }
    handleMousePressed({position, event.pointerIndex});
 }

void ASelectableLabel::onPointerReleased(const APointerReleasedEvent& event) {
    AView::onPointerReleased(event);
    auto position = event.position;
    position.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == ATextAlign::CENTER) {
        position.x += mPrerendered->getWidth() / 2.f;
    }
    handleMouseReleased({position, event.pointerIndex});
}

void ASelectableLabel::onFocusLost() {
    AView::onFocusLost();
    clearSelection();
    redraw();
}

void ASelectableLabel::onKeyDown(AInput::Key key) {
    AView::onKeyDown(key);

    if (AInput::isKeyDown(AInput::LCONTROL) || AInput::isKeyDown(AInput::RCONTROL))
        switch (key) {
            case AInput::A:
                selectAll();
                break;

            case AInput::X:
            case AInput::C:
                if (hasSelection())
                    AClipboard::copyToClipboard(selectedText());
                break;
    }
}

bool ASelectableLabel::consumesClick(const glm::ivec2& pos) {
    return true;
}

void ASelectableLabel::onFocusAcquired() {
    AView::onFocusAcquired();
}


void ASelectableLabel::onPointerDoubleClicked(const APointerPressedEvent& event) {
    AView::onPointerDoubleClicked(event);
    ACursorSelectable::handleMouseDoubleClicked(event);
}

size_t ASelectableLabel::textLength() const {
    return ALabel::text().length();
}
