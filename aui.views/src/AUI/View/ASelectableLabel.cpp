// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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

void ASelectableLabel::render() {
    AView::render();

    if (hasFocus()) {
        auto x =  mTextLeftOffset;
        if (getFontStyleLabel().align == TextAlign::CENTER) {
            x -= mPrerendered->getWidth() / 2.f;
        }
        {
            RenderHints::PushMatrix m;

            Render::setTransform(glm::translate(glm::mat4(1.f), {x, 0, 0}));
            drawSelectionPre();
        }
        doRenderText();

        {
            RenderHints::PushMatrix m;
            Render::setTransform(glm::translate(glm::mat4(1.f), {x, 0, 0}));
            drawSelectionPost();
        }
    } else {
        doRenderText();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ASelectableLabel::onMouseMove(glm::ivec2 pos) {
    AView::onMouseMove(pos);
    pos.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == TextAlign::CENTER) {
        pos.x += mPrerendered->getWidth() / 2.f;
    }
    handleMouseMove(pos);
}

void ASelectableLabel::onMousePressed(const APointerPressedEvent& event) {
    clearSelection();
    AView::onMousePressed(event);
    auto position = event.position;
    position.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == TextAlign::CENTER) {
        position.x += mPrerendered->getWidth() / 2.f;
    }
    handleMousePressed({position, event.button});
 }

void ASelectableLabel::onMouseReleased(const APointerReleasedEvent& event) {
    AView::onMouseReleased(event);
    auto position = event.position;
    position.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == TextAlign::CENTER) {
        position.x += mPrerendered->getWidth() / 2.f;
    }
    handleMouseReleased({position, event.button});
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


void ASelectableLabel::onMouseDoubleClicked(const APointerPressedEvent& event) {
    AView::onMouseDoubleClicked(event);
    ACursorSelectable::handleMouseDoubleClicked(event);
}

size_t ASelectableLabel::textLength() const {
    return ALabel::text().length();
}
