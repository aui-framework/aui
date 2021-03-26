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

#pragma once

#include <AUI/Model/AModelSelection.h>
#include "AViewContainer.h"
#include "AUI/Model/AModelIndex.h"
#include "AUI/Model/IListModel.h"
#include "AScrollbar.h"

class AListItem;
class AListViewContainer;

class API_AUI_VIEWS AListView: public AViewContainer
{
private:
    _<AListViewContainer> mContent;
    _<AScrollbar> mScrollbar;
	_<IListModel<AString>> mModel;
	ASet<AModelIndex> mSelectionModel;

    void updateScrollbarDimensions();
	
public:
    AListView(): AListView(nullptr) {}
	explicit AListView(const _<IListModel<AString>>& model);
    virtual ~AListView();

    void setModel(const _<IListModel<AString>>& model);

    int getContentMinimumHeight() override;
    int getContentFullHeight() {
        return getLayout()->getMinimumHeight() + 8;
    }
	
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

	[[nodiscard]] AModelSelection<AString> getSelectionModel() const {
	    return AModelSelection<AString>(mSelectionModel, mModel.get());
	}

    void setSize(int width, int height) override;

    void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) override;

    void onMouseWheel(glm::ivec2 pos, int delta) override;

signals:
	emits<AModelSelection<AString>> selectionChanged;
	emits<unsigned> itemDoubleClicked;

};
