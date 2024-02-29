// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "AScrollAreaInner.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/View/AView.h"

void AScrollAreaInner::setContents(_<AView> content) {
    if (mContents) {
        removeView(mContents);
    }
    content->setPosition({0, 0});
    if (content) addView(content);
    mContents = std::move(content);
    updateContentsScroll();
}

void AScrollAreaInner::updateLayout() {
    AViewContainer::updateLayout();
    AUI_NULLSAFE(mContents)->setSize(glm::max(mContents->getMinimumSize(), getSize()));
}
void AScrollAreaInner::updateContentsScroll() {
    AUI_NULLSAFE(mContents)->setPosition(-glm::ivec2(mScroll));
}