/**
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AListView.h"
#include "ALabel.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/Platform/AWindow.h"



class AListViewContainer: public AViewContainer {
private:
    int mScrollY = 0;

public:
    void updateLayout() override {
        if (getLayout())
            getLayout()->onResize(mPadding.left, mPadding.top - mScrollY,
                                  getSize().x - mPadding.horizontal(), getSize().y - mPadding.vertical());
        updateParentsLayoutIfNecessary();
    }

    void setScrollY(int scrollY) {
        mScrollY = scrollY;
        updateLayout();
    }
};

class AListItem: public ALabel
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


	void getCustomCssAttributes(AMap<AString, AVariant>& map) override
	{
		ALabel::getCustomCssAttributes(map);
		if (mSelected)
			map["selected"] = true;
	}

	void setSelected(const bool selected)
	{
		mSelected = selected;
		emit customCssPropertyChanged;
	}
};



int AListView::getContentMinimumHeight()
{
	return 40;
}

void AListView::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
	AViewContainer::onMousePressed(pos, button);

	auto target = _cast<AListItem>(getViewAt(pos));
	if (target) {
	    if (!AInput::isKeyDown(AInput::LControl)) {
	        for (auto& s : mSelectionModel) {
                _cast<AListItem>(getViews()[s.getRow()])->setSelected(false);
	        }

	        mSelectionModel.clear();
	    }
	    mSelectionModel << AModelIndex(getViews().indexOf(target));
	    target->setSelected(true);

        emit selectionChanged(getSelectionModel());
	}
}

void AListView::onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMouseDoubleClicked(pos, button);

    auto target = _cast<AListItem>(getViewAt(pos));
    if (target) {
        emit itemDoubleClicked(getViews().indexOf(target));
    }
}

AListView::~AListView()
{
}

AListView::AListView(const _<IListModel<AString>>& model) {
    setModel(model);
    updateScrollbarDimensions();
}

void AListView::setModel(const _<IListModel<AString>>& model) {
    mModel = model;
    setLayout(_new<AHorizontalLayout>());

    addView(mContent = _new<AListViewContainer>());
    addView(mScrollbar = _new<AScrollbar>());

    mContent->setLayout(_new<AVerticalLayout>());
    mContent->setExpanding({2, 2});
    //mScrollbar->setVisibility(Visibility::GONE);

    connect(mScrollbar->scrolled, mContent, &AListViewContainer::setScrollY);

    if (mModel) {
        for (size_t i = 0; i < model->listSize(); ++i) {
            mContent->addView(_new<AListItem>(model->listItemAt(i)));
        }

        connect(mModel->dataInserted, this, [&](const AModelRange<AString>& data) {
            for (const auto& row : data) {
                mContent->addView(_new<AListItem>(row));
            }
            updateLayout();
            redraw();
        });
        connect(mModel->dataChanged, this, [&](const AModelRange<AString>& data) {
            for (size_t i = data.getBegin().getRow(); i != data.getEnd().getRow(); ++i) {
                _cast<AListItem>(mContent->getViews()[i])->setText(data.getModel()->listItemAt(i));
            }
            redraw();
        });
        connect(mModel->dataRemoved, this, [&](const AModelRange<AString>& data) {
            size_t index = data.getBegin().getRow();
            for (size_t i = data.getBegin().getRow(); i < data.getEnd().getRow(); ++i) {
                mContent->removeView(mViews[index]);
            }
            updateLayout();
            redraw();
        });
    }
    updateLayout();
    AWindow::current()->flagRedraw();
}

void AListView::setSize(int width, int height) {
    AViewContainer::setSize(width, height);

    updateScrollbarDimensions();
}

void AListView::updateScrollbarDimensions() {
    mScrollbar->setScrollDimensions(getHeight(), mContent->getContentMinimumHeight());
}

void AListView::onMouseWheel(glm::ivec2 pos, int delta) {
    //AViewContainer::onMouseWheel(pos, delta);
    mScrollbar->onMouseWheel(pos, delta);
}
