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
// Created by Alex2772 on 7/14/2022.
//

#include "AGroupBox.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "ACheckBox.h"

using namespace declarative;

namespace {
    class Inner: public AViewContainerBase {
    friend class AGroupBox;
    public:
        Inner(_<AView> title, const _<AViewContainer>& contents) : mTitle(std::move(title)) {
            setContents(contents);
        }

        void drawStencilMask(ARenderContext ctx) override {
            AView::drawStencilMask(ctx);

            RenderHints::PushMatrix transform(ctx.render);
            auto d = mTitle->getPositionInWindow() - getPositionInWindow();
            AUI_REPEAT(2) { // render twice to definitely avoid stencil issues
                ctx.render.rectangle(ASolidBrush{},
                                     d,
                                     mTitle->getSize());
            }
        }

    private:
        _<AView> mTitle;
    };
}

AGroupBox::AGroupBox(_<AView> titleView, _<AView> contentView):
    mTitle(std::move(titleView)),
    mContent(std::move(contentView)) {


    setLayout(std::make_unique<AVerticalLayout>());

    using namespace declarative;
    setContents(Vertical {
        Horizontal {
            mTitle AUI_LET {
                if (auto label = _cast<ALabel>(it)) {
                    label->setOverflow(AOverflow::HIDDEN);
                    label->setTextOverflow(ATextOverflow::ELLIPSIS);
                }
            }
        } << ".agroupbox-title",
        mFrame = _new<Inner>(mTitle,
                            /*
                             * Using two nested container because view's masking does not affect it's background (style), but does for
                             * its children.
                             */
                             Vertical {
                                 Vertical::Expanding {
                                     mContent AUI_LET { it->setExpanding(); }
                                 }  << ".agroupbox-inner"
                             } AUI_WITH_STYLE {
                                     Expanding {},
                                     AOverflow::HIDDEN, // forces to call drawStencilMask
                             }) AUI_LET {
        },
    });

    if (auto asCheckbox = _cast<ACheckBox>(mTitle)) {
        connect(asCheckbox->checked(), me::updateCheckboxState);
    } else if (auto asCheckbox = _cast<ACheckBoxWrapper>(mTitle)) {
        connect(asCheckbox->checked(), me::updateCheckboxState);
    }
}

void AGroupBox::applyGeometryToChildren() {
    AViewContainerBase::applyGeometryToChildren();
    mFrame->setGeometry({mFrame->getPosition().x, getFrameForcedPosition()}, mFrame->getSize());
}

int AGroupBox::getFrameForcedPosition() const noexcept {
    return mTitle->getPosition().y + mTitle->getSize().y / 2;
}

void AGroupBox::updateCheckboxState(bool checked) {
    mFrame->setEnabled(checked);
}
