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


#include "AViewContainer.h"
#include "AUI/Util/Declarative.h"

/**
 * @brief A titled frame
 * @ingroup useful_views
 * @details
 * Group box provides a frame and a customizable title on top.
 *
 * If the title is a checkbox then it's checked state used to enable/disable the elements of this group box.
 *
 * Frame of the groupbox cam be customized by `".agroupbox-inner"` class as like the title is customized by
 * `".agroupbox-title"` class.
 *
 * Frame of the groupbox is not drawn under the title; this behaviour can be overridden by the Overflow::VISIBLE style
 * rule applied for `".agroupbox-inner"`.
 *
 * Title is done by hacking the vertical layout and forcing the frame top side to the center of the title view.
 */
class API_AUI_VIEWS AGroupBox: public AViewContainer {
public:
    AGroupBox(_<AView> titleView, _<AView> contentView);

    void updateLayout() override;

    int getContentMinimumHeight(ALayoutDirection layout) override;

private:
    _<AView> mTitle;
    _<AView> mContent;
    _<AView> mFrame;

    int getFrameForcedPosition() const noexcept;

    void updateCheckboxState(bool checked);
};



namespace declarative {
    using GroupBox = aui::ui_building::view<AGroupBox>;
}
