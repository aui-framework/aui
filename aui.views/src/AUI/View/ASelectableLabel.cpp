/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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

void ASelectableLabel::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    clearSelection();
    AView::onMousePressed(pos, button);
    pos.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == TextAlign::CENTER) {
        pos.x += mPrerendered->getWidth() / 2.f;
    }
    handleMousePressed(pos, button);
 }

void ASelectableLabel::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AView::onMouseReleased(pos, button);
    pos.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == TextAlign::CENTER) {
        pos.x += mPrerendered->getWidth() / 2.f;
    }
    handleMouseReleased(pos, button);
}

void ASelectableLabel::onFocusLost() {
    AView::onFocusLost();
    clearSelection();
    redraw();
}

void ASelectableLabel::onKeyDown(AInput::Key key) {
    AView::onKeyDown(key);

    if (AInput::isKeyDown(AInput::LControl) || AInput::isKeyDown(AInput::RControl))
        switch (key) {
            case AInput::A:
                selectAll();
                break;

            case AInput::X:
            case AInput::C:
                if (hasSelection())
                    AClipboardImpl::copyToClipboard(getSelectedText());
                break;
    }
}

bool ASelectableLabel::consumesClick(const glm::ivec2& pos) {
    return true;
}

void ASelectableLabel::onFocusAcquired() {
    AView::onFocusAcquired();
}


void ASelectableLabel::onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) {
    AView::onMouseDoubleClicked(pos, button);
    ACursorSelectable::handleMouseDoubleClicked(pos, button);
}

size_t ASelectableLabel::getTextLength() const {
    return ALabel::getText().length();
}
