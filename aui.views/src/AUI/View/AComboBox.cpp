/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 21.09.2020.
//

#include <AUI/Util/Cache.h>
#include "AComboBox.h"
#include "AListView.h"
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Platform/AWindow.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Animator/ATranslationAnimator.h>
#include <AUI/Animator/ASizeAnimator.h>
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/ASS/ASS.h>

AComboBox::AComboBox(const _<IListModel<AString>>& model):
        AComboBox()
{
    setModel(model);
}

AComboBox::AComboBox() {

}

void AComboBox::setModel(const _<IListModel<AString>>& model) {
    mModel = model;
    setSelectionId(0);

    connect(model->dataInserted, &AComboBox::updateText);
    connect(model->dataRemoved,  &AComboBox::updateText);
    connect(model->dataChanged,  &AComboBox::updateText);
}

void AComboBox::setSelectionId(int id) {
    mSelectionId = id;
    if (mModel->listSize() > id) {
        updateText();
    }
    emit selectionChanged(id);
}

void AComboBox::updateText() {
    if (mModel->listSize() == 0) {
        redraw();
        return;
    }
    if (mModel->listSize() <= mSelectionId) {
        mSelectionId = 0;
    }
    setText(mModel->listItemAt(mSelectionId));
}
void AComboBox::render() {
    AAbstractLabel::render();
    if (auto arrow = IDrawable::fromUrl(":uni/svg/combo.svg")) {
        auto size = arrow->getSizeHint();
        IDrawable::Params p;
        p.size = size;
        p.offset = { getWidth() - size.x - getPadding().right, (getHeight() - size.y) / 2 };
        arrow->draw(p);
    }
}

int AComboBox::getContentMinimumWidth(ALayoutDirection layout) {
    return AAbstractLabel::getContentMinimumWidth(ALayoutDirection::NONE) + 20;
}

void AComboBox::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AView::onMouseReleased(pos, button);

    if (!mComboWindow.lock()) {
        auto parentWindow = getWindow();
        if (!parentWindow) return;

        auto list = _new<AListView>(mModel) with_style { ass::Margin { 0 }, ass::Expanding{}, ass::MinSize {  AMetric(getWidth(), AMetric::T_PX), 0, } };
        list << ".combobox_list";
        int listHeight = list->getContentFullHeight() + list->getMinimumHeight() + 2; // bias
        auto comboBoxPos = getPositionInWindow();
        unsigned usedPositionIndex;
        auto comboWindow = parentWindow->createOverlappingSurface(
                [&](unsigned attempt) -> std::optional<glm::ivec2> {
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
        comboWindow->setLayout(_new<AVerticalLayout>());
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
        connect(list->selectionChanged, this, [&](const AModelSelection<AString>& s) {
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

AComboBox::~AComboBox() {
    if (auto l = mComboWindow.lock())
        l->close();
}

void AComboBox::destroyWindow() {
    mPopup = false;
    if (auto l = mComboWindow.lock()) {
        l->close();
    }
    mComboWindow.reset();
    emit customCssPropertyChanged();

    redraw();
}

void AComboBox::onComboBoxWindowCreated() {

}

