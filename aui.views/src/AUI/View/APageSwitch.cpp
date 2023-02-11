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

//
// Created by alex2 on 27.10.2020.
//

#include "APageSwitch.h"

APageSwitch::APageSwitch(const AString& text, unsigned pageNumber, const _<APageView>& pageView):
    ALabel(text),
    mPageNumber(pageNumber),
    mPageView(pageView)
{

    connect(pageView->pageChanged, this, [&]() {
        emit customCssPropertyChanged();
    });
    connect(clicked, this, [pageView, pageNumber]() {
        pageView->setPageId(pageNumber);
    });
}

int APageSwitch::getContentMinimumWidth(ALayoutDirection layout) {
    return ALabel::getContentMinimumWidth(ALayoutDirection::NONE);
}
