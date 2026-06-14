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
#include <AUI/View/ACheckBox.h>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/ITexture.h>
#include <AUI/Render/IRendererBackend.h>

using namespace declarative;

class AGroupBoxInner : public AViewContainerBase {
public:
    AGroupBoxInner(_<AView> title, _<AViewContainer> contents) : mTitle(std::move(title)) {
        setContents(std::move(contents));
    }

    void render(ARenderContext ctx) override {
        if (mTitle->getVisibility() != Visibility::GONE) {
            auto titleRect = ARect<float>::fromTopLeftPositionAndSize(mTitle->getPositionInWindow(),
                                                                      mTitle->getSize());
            titleRect.p1 -= getPositionInWindow();
            titleRect.p2 -= getPositionInWindow();

            ctx.canvas.pushClipRect(titleRect, AClipOp::OP_DIFFERENCE);
            AViewContainerBase::render(ctx);
            ctx.canvas.popClipRect();
        } else {
            AViewContainerBase::render(ctx);
        }
    }

private:
    _<AView> mTitle;
};

AGroupBox::AGroupBox(_<AView> titleView, _<AView> contentView) :
    mTitle(std::move(titleView)),
    mContent(std::move(contentView)) {
    setLayout(std::make_unique<AVerticalLayout>());

    using namespace declarative;
    setContents(Vertical{
        Horizontal{ mTitle } << ".agroupbox-title",
        mFrame = _new<AGroupBoxInner>(mTitle,
                                      Vertical{
                                          Vertical::Expanding{
                                              mContent AUI_LET { it->setExpanding(); }
                                          } << ".agroupbox-inner"
                                      } AUI_OVERRIDE_STYLE{
                                          Expanding{},
                                          AOverflow::HIDDEN, // forces to call resolveMasks
                                      })
    });

    if (auto asCheckbox = _cast<ACheckBox>(mTitle)) {
        connect(asCheckbox->checked(), me::updateCheckboxState);
    } else if (auto asCheckbox = _cast<ACheckBox::Box>(mTitle)) {
        connect(asCheckbox->checked, me::updateCheckboxState);
    }
}

void AGroupBox::applyGeometryToChildren() {
    AViewContainerBase::applyGeometryToChildren();
}

void AGroupBox::updateCheckboxState(bool checked) {
    mFrame->setEnabled(checked);
}
