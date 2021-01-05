/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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

#include "AListView.h"
#include "ALabel.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/Platform/AWindow.h"



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

AListView::~AListView()
{
}

AListView::AListView(const _<IListModel<AString>>& model) {

    setModel(model);
}

void AListView::setModel(const _<IListModel<AString>>& model) {
    mModel = model;
    setLayout(_new<AVerticalLayout>());

    if (mModel) {
        for (size_t i = 0; i < model->listSize(); ++i) {
            addView(_new<AListItem>(model->listItemAt(i)));
        }

        connect(mModel->dataInserted, this, [&](const AModelRange<AString>& data) {
            for (const auto& row : data) {
                addView(_new<AListItem>(row));
            }
            updateLayout();
        });
    }
    updateLayout();
    AWindow::current()->flagRedraw();
}

void AListView::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
}
