//
// Created by Alex2772 on 7/14/2022.
//

#include "AGroupBox.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "ACheckBox.h"

using namespace declarative;

namespace {
    class Inner: public AViewContainer {
    friend class AGroupBox;
    public:
        Inner(_<AView> title, const _<AViewContainer>& contents) : mTitle(std::move(title)) {
            setContents(contents);
        }

        void drawStencilMask() override {
            AView::drawStencilMask();

            RenderHints::PushMatrix transform;
            auto d = mTitle->getPositionInWindow() - getPositionInWindow();
            Render::rect(ASolidBrush{},
                         d,
                         mTitle->getSize());
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
        mFrame = _new<Inner>(mTitle,
                            /*
                             * Using two nested container because view's masking does not affect it's background (style), but does for
                             * it's children.
                             */
                             Vertical {
                                 Vertical::Expanding {
                                     mContent let { it->setExpanding(); }
                                 }  << ".agroupbox-inner"
                             } with_style {
                                     Expanding {},
                                     AOverflow::HIDDEN, // forces to call drawStencilMask
                             }) let {
        },
    });

    if (auto asCheckbox = _cast<ACheckBox>(mTitle)) {
        connect(asCheckbox->checked, me::updateCheckboxState);
        updateCheckboxState(asCheckbox->isChecked());
    } else if (auto asCheckbox = _cast<ACheckBoxWrapper>(mTitle)) {
        connect(asCheckbox->checked, me::updateCheckboxState);
        updateCheckboxState(asCheckbox->isChecked());
    }
}

void AGroupBox::updateLayout() {
    AViewContainer::updateLayout();

    mFrame->setGeometry({mFrame->getPosition().x, getFrameForcedPosition()}, mFrame->getSize());
}

int AGroupBox::getContentMinimumHeight(ALayoutDirection layout) {
    return AViewContainer::getContentMinimumHeight(ALayoutDirection::NONE) + mFrame->getPosition().y - getFrameForcedPosition();
}

int AGroupBox::getFrameForcedPosition() const noexcept {
    return mTitle->getPosition().y + mTitle->getSize().y / 2;
}

void AGroupBox::updateCheckboxState(bool checked) {
    mFrame->setEnabled(checked);
}
