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
// Created by alex2772 on 6/27/21.
//

#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "ARulerArea.h"
#include "ARulerView.h"
#include "AScrollbar.h"
#include <AUI/ASS/ASS.h>

using namespace ass;

class WrapperContainer: public AViewContainerBase {
private:
    _<AView> mWrappedView;

public:
    WrapperContainer(const _<AView>& wrappedView) : mWrappedView(wrappedView) {
        addView(mWrappedView);
    }

};

ARulerArea::ARulerArea(const _<AView>& wrappedView) : mWrappedView(wrappedView) {

    auto wrapper = _new<WrapperContainer>(mWrappedView) << ".arulerarea-content" AUI_WITH_STYLE { Expanding { 2, 2 } };

    setContents(
        Horizontal {
            Vertical {
                _container<AAdvancedGridLayout>({
                    _new<ALabel>("dp") << ".arulerarea-unit",
                    mHorizontalRuler = _new<ARulerView>(ALayoutDirection::HORIZONTAL),
                    mVerticalRuler = _new<ARulerView>(ALayoutDirection::VERTICAL),
                    wrapper AUI_WITH_STYLE {AOverflow::HIDDEN },
                }, 2, 2) AUI_WITH_STYLE { Expanding { 2, 2 } },
                _new<AScrollbar>(ALayoutDirection::HORIZONTAL),

            } AUI_WITH_STYLE { Expanding { 2, 2 } },
            _new<AScrollbar>(),
        }
    );

    setExpanding({10, 10});


    mWrappedView->setSize({ mWrappedView->getMinimumWidth(), mWrappedView->getMinimumHeight() });
}

void ARulerArea::setSize(glm::ivec2 size) {
    AViewContainerBase::setSize(size);
    mWrappedView->setSize({ mWrappedView->getMinimumWidth(), mWrappedView->getMinimumHeight() });
    updatePosition();
}

void ARulerArea::setWrappedViewPosition(const glm::ivec2& pos) {
    mWrappedView->setPosition(pos);
    mHorizontalRuler->setOffsetPx(pos.x);
    mVerticalRuler->setOffsetPx(pos.y);
}

void ARulerArea::render(ARenderContext ctx) {
    AViewContainerBase::render(ctx);


    glDisable(GL_STENCIL_TEST);
    // cursor display
    const auto rulerOffset = glm::ivec2(mVerticalRuler->getWidth(), mHorizontalRuler->getHeight());
    if (glm::any(glm::greaterThan(mMousePos, rulerOffset * 2))) {
        mMousePos = glm::clamp(mMousePos, rulerOffset, getSize() - rulerOffset);

        glm::ivec2 tp = mMousePos - (getTargetPosition() + rulerOffset);

        AFontStyle fs = getFontStyle();
        auto prX = ctx.render.prerenderString({0, 0 }, AString::number(int(operator ""_px(tp.x).getValueDp())), fs);
        auto prY = ctx.render.prerenderString({0, 0 }, AString::number(int(operator ""_px(tp.y).getValueDp())), fs);

        glm::vec2 maxNumbersPos = glm::vec2(getSize() - rulerOffset) - glm::vec2(prX->getWidth(), fs.size) - glm::vec2(4_dp);

        {
            RenderHints::PushMatrix m(ctx.render);
            ctx.render.translate({glm::min(mMousePos.x + 2_dp, maxNumbersPos.x), 18_dp });
            prX->draw();
        }
        {
            RenderHints::PushMatrix m(ctx.render);
            ctx.render.translate({18_dp, glm::min(mMousePos.y + 2_dp, maxNumbersPos.y) });
            prY->draw();
        }

        ctx.render.setBlending(Blending::INVERSE_DST);
        ctx.render.rectangle(ASolidBrush{},
                             {mMousePos.x, 0.f},
                             {1, mMousePos.y});
        ctx.render.rectangle(ASolidBrush{},
                             {0.f, mMousePos.y},
                             {mMousePos.x, 1});


        ctx.render.setBlending(Blending::NORMAL);
    }

    glEnable(GL_STENCIL_TEST);
}

void ARulerArea::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AViewContainerBase::onPointerMove(pos, event);
    mMousePos = pos;
    redraw();
}

void ARulerArea::updatePosition() {
    setWrappedViewPosition(getTargetPosition());
}

glm::ivec2 ARulerArea::getTargetPosition() const {
    return (getSize() - mWrappedView->getSize()) / 2;
}

void ARulerArea::invalidateFont() {

}
