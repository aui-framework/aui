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
            ctx.render.rectangle(ASolidBrush{},
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
