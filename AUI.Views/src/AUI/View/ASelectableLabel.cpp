//
// Created by alex2 on 30.11.2020.
//

#include <AUI/Platform/AClipboard.h>
#include "ASelectableLabel.h"
#include <AUI/GL/gl.h>

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
        drawSelectionPre();
        doRenderText();
        drawSelectionPost();
    } else {
        doRenderText();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ASelectableLabel::onMouseMove(glm::ivec2 pos) {
    AView::onMouseMove(pos);
    handleMouseMove(pos);
}

void ASelectableLabel::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    AView::onMousePressed(pos, button);
    handleMousePressed(pos, button);
 }

void ASelectableLabel::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AView::onMouseReleased(pos, button);
    handleMouseReleased(pos, button);
}

void ASelectableLabel::onFocusLost() {
    AView::onFocusLost();
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
