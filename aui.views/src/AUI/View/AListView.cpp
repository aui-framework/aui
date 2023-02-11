// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AListView.h"
#include "ALabel.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/Platform/AWindow.h"



class AListViewContainer: public AViewContainer {
private:
    int mScrollY = 0;
    size_t mIndex = -1;

public:
    void updateLayout() override {
        if (getLayout())
            getLayout()->onResize(mPadding.left, mPadding.top - mScrollY,
                                  getSize().x - mPadding.horizontal(), getSize().y - mPadding.vertical());
        updateParentsLayoutIfNecessary();
    }

    _<AView> getViewAt(glm::ivec2 pos, bool ignoreGone) override {
        switch (mViews.size()) {
            case 0:
                return nullptr;
            case 1: {
                auto v = AViewContainer::getViewAt(pos, ignoreGone);
                mIndex = v ? 0 : -1;
                return v;
            }
            default: {
                mIndex = -1;
                pos.y += mScrollY;
                pos.y /= mViews[1]->getPosition().y - mViews[0]->getPosition().y;
                if (pos.y >= 0 && pos.y < mViews.size()) {
                    return mViews[mIndex = pos.y];
                }
                return nullptr;
            }
        }
    }

    void setScrollY(int scrollY) {
        mScrollY = scrollY;
        updateLayout();
    }

    size_t getIndex() const {
        return mIndex;
    }

    int getContentMinimumHeight(ALayoutDirection layout) override {
        return 40;
    }
};

class AListItem: public ALabel, public ass::ISelectable
{
private:
	bool mSelected = false;

public:
	AListItem()
	{

	}

	AListItem(const AString& text)
		: ALabel(text)
	{
	}

	virtual ~AListItem() = default;

	void setSelected(const bool selected)
	{
		mSelected = selected;
		emit customCssPropertyChanged;
	}

protected:
    bool selectableIsSelectedImpl() override {
        return mSelected;
    }

public:

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override {
        AView::onMousePressed(pos, button);

        dynamic_cast<AListView*>(getParent()->getParent())->handleMousePressed(this);
    }

    void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) override {
        AView::onMouseDoubleClicked(pos, button);

        dynamic_cast<AListView*>(getParent()->getParent())->handleMouseDoubleClicked(this);
    }
};



int AListView::getContentMinimumHeight(ALayoutDirection layout)
{
	return 40;
}


AListView::~AListView()
{
}

AListView::AListView(const _<IListModel<AString>>& model) {
    mObserver = _new<AListModelObserver<AString>>(this);
    setModel(model);
}

void AListView::setModel(const _<IListModel<AString>>& model) {
    setLayout(_new<AHorizontalLayout>());

    addView(mContent = _new<AListViewContainer>());
    addView(mScrollbar = _new<AScrollbar>());

    mContent->setLayout(_new<AVerticalLayout>());
    mContent->setExpanding();

    connect(mScrollbar->scrolled, mContent, &AListViewContainer::setScrollY);
    mObserver->setModel(model);
    if (model) {
        connect(model->dataRemoved, [&] {
            mSelectionModel.clear();
        });
    }
}

void AListView::setSize(glm::ivec2 size) {
    AViewContainer::setSize(size);

    updateScrollbarDimensions();
}

void AListView::updateScrollbarDimensions() {
    if (!mScrollbar) return;
    mScrollbar->setScrollDimensions(getHeight(), mContent->AViewContainer::getContentMinimumHeight(
            ALayoutDirection::NONE));
}

void AListView::onMouseWheel(glm::ivec2 pos, glm::ivec2 delta) {
    if (!mScrollbar) return;
    //AViewContainer::onMouseWheel(pos, delta);
    mScrollbar->onMouseWheel(pos, delta);
    onMouseMove(pos); // update hover on scroll
}

void AListView::handleMousePressed(AListItem* item) {

    if (!AInput::isKeyDown(AInput::LCONTROL) || !mAllowMultipleSelection) {
        clearSelection();
    }
    mSelectionModel << AModelIndex(mContent->getIndex());
    item->setSelected(true);

    emit selectionChanged(getSelectionModel());
}

void AListView::clearSelection() {
    for (auto& s : mSelectionModel) {
        _cast<AListItem>(mContent->getViews()[s.getRow()])->setSelected(false);
    }

    mSelectionModel.clear();
}

void AListView::handleMouseDoubleClicked(AListItem* item) {
    emit itemDoubleClicked(mContent->getIndex());
}

void AListView::insertItem(size_t at, const AString& value) {
    mContent->addView(at, _new<AListItem>(value));
}

void AListView::updateItem(size_t at, const AString& value) {
    _cast<AListItem>(mContent->getViews()[at])->setText(value);
}

void AListView::removeItem(size_t at) {
    mContent->removeView(at);
}

void AListView::onDataCountChanged() {
    updateLayout();
    updateScrollbarDimensions();
}

void AListView::onDataChanged() {
    redraw();
}

void AListView::selectItem(size_t i) {
    clearSelection();
    mSelectionModel = { AModelIndex(i) };
    _cast<AListItem>(mContent->getViews()[i])->setSelected(true);
}

bool AListView::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    if (std::holds_alternative<AFingerDragEvent>(event)) {
        if (transformGestureEventsToDesktop(origin, event)) {
            return true;
        }
    }
    return AViewContainer::onGesture(origin, event);
}

void AListView::setAllowMultipleSelection(bool allowMultipleSelection) {
    mAllowMultipleSelection = allowMultipleSelection;
    clearSelection();
}
