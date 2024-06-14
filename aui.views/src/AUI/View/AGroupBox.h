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
