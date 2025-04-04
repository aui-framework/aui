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
// Created by alex2 on 21.09.2020.
//

#include <AUI/Util/Cache.h>
#include "ADropdownList.h"
#include "AListView.h"
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Platform/AWindow.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Animator/ATranslationAnimator.h>
#include <AUI/Animator/ASizeAnimator.h>
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/ASS/ASS.h>

ADropdownList::ADropdownList(const _<IListModel<AString>>& model):
        ADropdownList()
{
    setModel(model);
}

ADropdownList::ADropdownList() {

}

void ADropdownList::setModel(const _<IListModel<AString>>& model) {
    mModel = model;
    mSelectionId = -1;
    setSelectionId(0);

    connect(model->dataInserted, me::updateText);
    connect(model->dataRemoved,  me::updateText);
    connect(model->dataChanged,  me::updateText);
}

void ADropdownList::setSelectionId(int id) {
    if (mSelectionId == id) {
        return;
    }
    mSelectionId = id;
    if (mModel->listSize() > id) {
        updateText();
    }
    emit selectionChanged(id);
}

void ADropdownList::updateText() {
    if (mModel->listSize() == 0) {
        redraw();
        return;
    }
    if (mModel->listSize() <= mSelectionId) {
        mSelectionId = 0;
    }
    setText(mModel->listItemAt(mSelectionId));
}
void ADropdownList::render(ARenderContext context) {
    AAbstractLabel::render(context);
    if (auto arrow = IDrawable::fromUrl(":uni/svg/combo.svg")) {
        auto size = arrow->getSizeHint();
        IDrawable::Params p;
        p.size = size;
        p.offset = { getWidth() - size.x - getPadding().right, (getHeight() - size.y) / 2 };
        arrow->draw(context.render, p);
    }
}

int ADropdownList::getContentMinimumWidth() {
    return AAbstractLabel::getContentMinimumWidth() + 20;
}

void ADropdownList::onPointerReleased(const APointerReleasedEvent& event) {
    AView::onPointerReleased(event);
    if (!event.triggerClick) return;

    if (!mComboWindow.lock()) {
        auto parentWindow = getWindow();
        if (!parentWindow) return;

        auto list = _new<AListView>(mModel) with_style { ass::Margin { 0 }, ass::Expanding{}, ass::MinSize {  AMetric(getWidth(), AMetric::T_PX), 0, } };
        list << ".combobox_list";
        int listHeight = list->getContentFullHeight() + list->getMinimumHeight() + 2; // bias
        auto comboBoxPos = getPositionInWindow();
        unsigned usedPositionIndex;
        auto comboWindow = parentWindow->createOverlappingSurface(
                [&](unsigned attempt) -> AOptional<glm::ivec2> {
                    usedPositionIndex = attempt;
                    switch (attempt) {
                        case 0: return comboBoxPos + glm::ivec2(0, getHeight()); // below
                        case 1: return comboBoxPos - glm::ivec2(0, listHeight - 1); // above
                        default: return std::nullopt;
                    }
                },{
                        (glm::max)(getWidth(), list->getMinimumWidth()),
                        listHeight
                });
        comboWindow->setLayout(std::make_unique<AVerticalLayout>());
        comboWindow->addView(list);
        mComboWindow = comboWindow;

        // when list floats up from below, we should apply only size animation
        if (usedPositionIndex == 0) {
            list->setAnimator(_new<ASizeAnimator>(
                    glm::ivec2{list->getWidth(), 0}) let { it->setDuration(0.15f); });
        } else {
            // when list floats up from above, we should apply both position and size animations
            list->setAnimator(AAnimator::combine({
                                                         _new<ATranslationAnimator>(glm::ivec2(0, listHeight)) let { it->setDuration(0.15f); },
                                                         _new<ASizeAnimator>(glm::ivec2{list->getWidth(), 0}) let { it->setDuration(0.15f); }
                                                 }));
        }
        connect(list->selectionChanged, this, [&](const AListModelSelection<AString>& s) {
            if (!s.getIndices().empty()) {
                setSelectionId(s.getIndices().begin()->getRow());
            }
            destroyWindow();
            AObject::disconnect();
        });
        onComboBoxWindowCreated();
        mPopup = true;
    } else {
        destroyWindow();
    }
}

ADropdownList::~ADropdownList() {
    if (auto l = mComboWindow.lock())
        l->close();
}

void ADropdownList::destroyWindow() {
    mPopup = false;
    if (auto l = mComboWindow.lock()) {
        l->close();
    }
    mComboWindow.reset();
    emit customCssPropertyChanged();

    redraw();
}

void ADropdownList::onComboBoxWindowCreated() {

}

