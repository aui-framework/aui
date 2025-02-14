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
#include "AUI/Views.h"

class AString;
class AWindow;

/**
 * @brief Displaying native modal message dialogs.
 * @ingroup views
 * @details
 * AMessageBox provides a set of APIs to display native modal message boxes, giving the user some important information
 * or asking questions.
 *
 * Title is a short sentence describing the situation in 2-4 words, typically rendered as emphasized text or in the
 * titlebar. Message contains more information about the state of things. Optionally, buttons or icon can be specified.
 *
 * The rest of the application is freezed/blocked until the user dismisses the message box. This means message boxes
 * should be used only in emergency situations, when your application literally can't operate until it recevies a
 * response from the user. Use non-blocking, contextual and structured feedback where possible.
 *
 * @snippet examples/ui/views/src/ExampleWindow.cpp AMessageBox
 *
 * @specificto{windows}
 * Implemented with @ref MessageBox.
 * @image html ajkdhadkjfh.png
 *
 * @specificto{linux}
 * Implemented with @ref GtkDialog.
 * @image html wjkfnjkws.png
 *
 * @specificto{macos}
 * Implemented with @ref NSAlert.
 * @image html revierio.png
 */
namespace AMessageBox {
enum class Icon { NONE, INFO, WARNING, CRITICAL };
enum class Button {
    OK,
    OK_CANCEL,
    YES_NO,
    YES_NO_CANCEL,
};
enum class ResultButton { INVALID, OK, CANCEL, YES, NO };
API_AUI_VIEWS ResultButton
show(AWindow* parent, const AString& title, const AString& message, Icon icon = Icon::NONE, Button b = Button::OK);
};   // namespace AMessageBox
