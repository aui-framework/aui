//
// Created by alex2772 on 6/27/21.
//

#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "ARulerArea.h"
#include "ARulerView.h"
#include "AScrollbar.h"
#include <AUI/ASS/ASS.h>

using namespace ass;

class WrapperContainer: public AViewContainer {
private:
    _<AView> mWrappedView;

public:
    WrapperContainer(const _<AView>& wrappedView) : mWrappedView(wrappedView) {
        addView(mWrappedView);
    }

};

ARulerArea::ARulerArea(const _<AView>& wrappedView) : mWrappedView(wrappedView) {

    auto wrapper = _new<WrapperContainer>(mWrappedView) with_style { Expanding { 2, 2 } } << ".arulerarea-content";

    setContents(
        Horizontal {
            Vertical {
                _container<AAdvancedGridLayout>({
                    _new<ALabel>("dp") << ".arulerarea-unit",
                    mHorizontalRuler = _new<ARulerView>(LayoutDirection::HORIZONTAL),
                    mVerticalRuler = _new<ARulerView>(LayoutDirection::VERTICAL),
                    wrapper with_style { Overflow::HIDDEN },
                }, 2, 2) with_style { Expanding { 2, 2 } },
                _new<AScrollbar>(LayoutDirection::HORIZONTAL),

            } with_style { Expanding { 2, 2 } },
            _new<AScrollbar>(),
        }
    );

    setExpanding({10, 10});


    mWrappedView->setSize(mWrappedView->getMinimumWidth(), mWrappedView->getMinimumHeight());
}

void ARulerArea::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
    mWrappedView->setSize(mWrappedView->getMinimumWidth(), mWrappedView->getMinimumHeight());
    updatePosition();
}

void ARulerArea::setWrappedViewPosition(const glm::ivec2& pos) {
    mWrappedView->setPosition(pos);
    mHorizontalRuler->setOffsetPx(pos.x);
    mVerticalRuler->setOffsetPx(pos.y);
}

void ARulerArea::render() {
    AViewContainer::render();


    glDisable(GL_STENCIL_TEST);
    // cursor display
    const auto rulerOffset = glm::ivec2(mVerticalRuler->getWidth(), mHorizontalRuler->getHeight());
    if (glm::any(glm::greaterThan(mMousePos, rulerOffset * 2))) {
        mMousePos = glm::clamp(mMousePos, rulerOffset, getSize() - rulerOffset);

        glm::ivec2 tp = mMousePos - (getTargetPosition() + rulerOffset);

        FontStyle fs = getFontStyle();
        fs.color = 0x0_rgb;
        auto prX = Render::preRendererString(AString::number(int(operator""_px(tp.x).getValueDp())), fs);
        auto prY = Render::preRendererString(AString::number(int(operator""_px(tp.y).getValueDp())), fs);

        glm::vec2 maxNumbersPos = glm::vec2(getSize() - rulerOffset) - glm::vec2(prX.length, fs.size) - glm::vec2(4_dp);

        Render::drawString(glm::min(mMousePos.x + 2_dp, maxNumbersPos.x), 18_dp, prX);
        Render::drawString(18_dp, glm::min(mMousePos.y + 2_dp, maxNumbersPos.y), prY);

        Render::setFill(Render::FILL_SOLID);
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        Render::drawRect(mMousePos.x, 0.f, 1, mMousePos.y);
        Render::drawRect(0.f, mMousePos.y, mMousePos.x, 1);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glEnable(GL_STENCIL_TEST);
}

void ARulerArea::onMouseMove(glm::ivec2 pos) {
    AViewContainer::onMouseMove(pos);
    mMousePos = pos;
    redraw();
}

void ARulerArea::updatePosition() {
    setWrappedViewPosition(getTargetPosition());
}

glm::ivec2 ARulerArea::getTargetPosition() const {
    return (getSize() - mWrappedView->getSize()) / 2;
}
