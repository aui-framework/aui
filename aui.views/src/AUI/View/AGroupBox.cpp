//
// Created by Alex2772 on 7/14/2022.
//

#include "AGroupBox.h"
#include "AUI/Util/UIBuildingHelpers.h"

using namespace declarative;

namespace {
    class Inner: public AViewContainer {
    friend class AGroupBox;
    public:
        Inner(_<AView> title) : mTitle(std::move(title)) {
        }

        void drawStencilMask() override {
            AView::drawStencilMask();

            RenderHints::PushMatrix transform;
            auto d = mTitle->getPositionInWindow() - getPositionInWindow();
            Render::translate(d);
            mTitle->drawStencilMask();
        }

    private:
        _<AView> mTitle;
    };
}

AGroupBox::AGroupBox(_<AView> titleView, _<AView> contentView):
    mTitle(std::move(titleView)),
    mContent(std::move(contentView)) {


    setLayout(_new<AVerticalLayout>());

    using namespace declarative;
    setContents(Vertical {
        Horizontal { mTitle } << ".agroupbox-title",
        mFrame = _new<Inner>(mTitle) let {
            /*
             * Using two nested container because view's masking does not affect it's background (style), but does for
             * it's children.
             */
            it->setContents(Vertical {
                Vertical::Expanding {
                    mContent let { it->setExpanding(); }
                }  << ".agroupbox-inner"
            } with_style {
                Expanding {},
                Overflow::HIDDEN, // forces to call drawStencilMas
            });
        },
    });
}

void AGroupBox::updateLayout() {
    AViewContainer::updateLayout();

    mFrame->setGeometry({mFrame->getPosition().x, getFrameForcedPosition()}, mFrame->getSize());
}

int AGroupBox::getContentMinimumHeight() {
    return AViewContainer::getContentMinimumHeight() + mFrame->getPosition().y - getFrameForcedPosition();
}

int AGroupBox::getFrameForcedPosition() const noexcept {
    return mTitle->getPosition().y + mTitle->getSize().y / 2;
}
