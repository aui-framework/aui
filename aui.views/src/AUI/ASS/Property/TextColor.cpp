/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 02.01.2021.
//

#include "TextColor.h"
#include "AUI/Font/IFontView.h"
#include "AUI/View/AViewContainer.h"

void ass::prop::Property<ass::TextColor>::applyFor(AView* view) {
    view->setTextColor(std::visit(
        aui::lambda_overloaded {
          [](AColor c) { return c; },
          [&](ass::inherit_t) {
              if (auto parent = view->getParent()) {
                  return parent->textColor();
              }
              return AColor();
          },
        },
        mInfo.color));
}