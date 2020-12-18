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

FontStyle ASelectableLabel::getMouseSelectionFont() {
    return getPrerendered().fs;
}

AString ASelectableLabel::getMouseSelectionText() {
    return getTargetText();
}

void ASelectableLabel::doRedraw() {
    redraw();
}

void ASelectableLabel::render() {
    AView::render();

    if (hasFocus()) {
        auto x =  mTextLeftOffset;
        if (getFontStyleLabel().align == ALIGN_CENTER) {
            x -= mPrerendered.length / 2.f;
        }
        {
            RenderHints::PushMatrix m;

            Render::inst().setTransform(glm::translate(glm::mat4(1.f), {x, 0, 0}));
            drawSelectionPre();
        }
        doRenderText();

        {
            RenderHints::PushMatrix m;
            Render::inst().setTransform(glm::translate(glm::mat4(1.f), {x, 0, 0}));
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
    if (getFontStyleLabel().align == ALIGN_CENTER) {
        pos.x += mPrerendered.length / 2.f;
    }
    handleMouseMove(pos);
}

void ASelectableLabel::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    AView::onMousePressed(pos, button);
    pos.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == ALIGN_CENTER) {
        pos.x += mPrerendered.length / 2.f;
    }
    handleMousePressed(pos, button);
 }

void ASelectableLabel::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AView::onMouseReleased(pos, button);
    pos.x -= mTextLeftOffset;
    if (getFontStyleLabel().align == ALIGN_CENTER) {
        pos.x += mPrerendered.length / 2.f;
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
                    AClipboard::copyToClipboard(getSelectedText());
                break;
    }
}

bool ASelectableLabel::consumesClick(const glm::ivec2& pos) {
    return true;
}

void ASelectableLabel::onFocusAcquired() {
    AView::onFocusAcquired();
}
