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

#include <AUI/Util/ALayoutDirection.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include "AViewContainer.h"
#include "AHDividerView.h"
#include "AVDividerView.h"
#include "ASplitterHelper.h"
#include <AUI/View/ASpacerExpanding.h>

/**
 * @brief A spinner view to display some long operation.
 * @ingroup useful_views
 * @details
 * A progress bar is used to express a long operation (i.e. file copy) with unknown progress and reassure the user that
 * application is still running.
 */
class API_AUI_VIEWS ASplitter: public AViewContainer
{
private:
    template<typename Layout>
    friend class Builder;

    ASplitterHelper mHelper;

    ASplitter();

    template<typename Layout>
    class Builder {
    friend class ASplitter;
    private:
        AVector<_<AView>> mItems;

    public:
       Builder& withItems(const AVector<_<AView>>& items) {
           mItems = items;
           return *this;
       }

       _<AView> build() {
           auto splitter = aui::ptr::manage(new ASplitter);
           splitter->mHelper.setDirection(Layout::DIRECTION);


           if (splitter->mHelper.mDirection == ALayoutDirection::VERTICAL) {
               splitter->setLayout(_new<AVerticalLayout>());
           } else {
               splitter->setLayout(_new<AHorizontalLayout>());
           }

           bool atLeastOneItemHasExpanding = false;
           for (auto& item : mItems) {
               splitter->addView(item);
               atLeastOneItemHasExpanding |= splitter->mHelper.getAxisValue(item->getExpanding()) > 0;
           }
           if (!atLeastOneItemHasExpanding) {
               auto spacer = _new<ASpacerExpanding>();
               splitter->addView(spacer);
               mItems << spacer;
           }

           splitter->mHelper.mItems = std::move(mItems);

           return splitter;
       }

       operator _<AView>() {
           return build();
       }
    };

public:
    virtual ~ASplitter() = default;

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseMove(glm::ivec2 pos) override;

    void setSize(int width, int height) override;

    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    using Vertical = Builder<AVerticalLayout>;
    using Horizontal = Builder<AHorizontalLayout>;

};
