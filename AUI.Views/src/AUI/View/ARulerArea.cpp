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

    void onMouseMove(glm::ivec2 pos) override {
        AViewContainer::onMouseMove(pos);
        emit mouseMove(pos);
    }
signals:
    emits<glm::ivec2> mouseMove;
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
                    wrapper,
                }, 2, 2) with_style { Expanding { 2, 2 } },
                _new<AScrollbar>(LayoutDirection::HORIZONTAL),

            } with_style { Expanding { 2, 2 } },
            _new<AScrollbar>(),
        }
    );

    connect(wrapper->mouseMove, [&](const glm::ivec2& pos) {
        mHorizontalRuler->setCursorPosPx(pos.x);
        mVerticalRuler->setCursorPosPx(pos.y);
    });

    setExpanding({10, 10});


    mWrappedView->setSize(mWrappedView->getMinimumWidth(), mWrappedView->getMinimumHeight());
}

void ARulerArea::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
    mWrappedView->setSize(mWrappedView->getMinimumWidth(), mWrappedView->getMinimumHeight());
    setWrappedViewPosition((getSize() - mWrappedView->getSize()) / 2);
}

void ARulerArea::setWrappedViewPosition(const glm::ivec2& pos) {
    mWrappedView->setPosition(pos);
    mHorizontalRuler->setOffsetPx(pos.x);
    mVerticalRuler->setOffsetPx(pos.y);
}
