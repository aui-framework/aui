/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

    void onPointerPressed(const APointerPressedEvent& event) override;

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;

    void setSize(glm::ivec2 size) override;

    void onPointerReleased(const APointerReleasedEvent& event) override;

    void onClickPrevented() override;

    using Vertical = Builder<AVerticalLayout>;
    using Horizontal = Builder<AHorizontalLayout>;

};
