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

#pragma once

#include <AUI/Model/AModelSelection.h>
#include <AUI/Model/AListModelObserver.h>
#include "AViewContainer.h"
#include "AUI/Model/AModelIndex.h"
#include "AUI/Model/IListModel.h"
#include "AScrollbar.h"

class AListItem;
class AListViewContainer;

/**
 * @brief Displays a list model of strings.
 * @ingroup views
 * @ingroup Views
 */
class API_AUI_VIEWS AListView: public AViewContainer, public AListModelObserver<AString>::IListModelListener
{
    friend class AListItem;
private:
    _<AListViewContainer> mContent;
    _<AScrollbar> mScrollbar;
	ASet<AModelIndex> mSelectionModel;
	_<AListModelObserver<AString>> mObserver;
    bool mAllowMultipleSelection = false;

    void updateScrollbarDimensions();
    void handleMousePressed(AListItem* item);
    void handleMouseDoubleClicked(AListItem* item);
	
public:
    AListView(): AListView(nullptr) {}
	explicit AListView(const _<IListModel<AString>>& model);
    virtual ~AListView();

    void setModel(const _<IListModel<AString>>& model);

    void selectItem(size_t i);

    int getContentMinimumHeight() override;
    int getContentFullHeight() {
        return getLayout()->getMinimumHeight() + 8;
    }

    void setAllowMultipleSelection(bool allowMultipleSelection);

    [[nodiscard]] AModelSelection<AString> getSelectionModel() const {
	    return AModelSelection<AString>(mSelectionModel, mObserver->getModel().get());
	}

    void setSize(int width, int height) override;
    void onMouseWheel(const glm::ivec2& pos, const glm::ivec2& delta) override;

    void insertItem(size_t at, const AString& value) override;
    void updateItem(size_t at, const AString& value) override;
    void removeItem(size_t at) override;

    void onDataCountChanged() override;
    void onDataChanged() override;

signals:
	emits<AModelSelection<AString>> selectionChanged;
	emits<unsigned> itemDoubleClicked;

    void clearSelection();

    bool onGesture(const glm::ivec2 &origin, const AGestureEvent &event) override;
};
