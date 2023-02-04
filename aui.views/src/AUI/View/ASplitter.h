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

    void setSize(glm::ivec2 size) override;

    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    using Vertical = Builder<AVerticalLayout>;
    using Horizontal = Builder<AHorizontalLayout>;

};
