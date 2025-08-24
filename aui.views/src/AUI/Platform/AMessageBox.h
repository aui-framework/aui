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
 * <!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp AMessageBox -->
 *
 * @specificto{windows}
 * Implemented with [MessageBox].
 * ![](imgs/ajkdhadkjfh.png)
 *
 * @specificto{linux}
 * Implemented with [GtkDialog].
 * ![](imgs/wjkfnjkws.png)
 *
 * @specificto{macos}
 * Implemented with [NSAlert].
 * ![](imgs/revierio.png)
 */
namespace AMessageBox {

/**
 * @brief Specifies icon to be displayed.
 */
enum class Icon {
    /**
     * @brief No icon is specified.
     * @details
     * @specificto{macos}
     * Displays app's bundle icon.
     */
    NONE,

    /**
     * @brief Information icon.
     */
    INFO,

    /**
     * @brief Warning icon.
     */
    WARNING,

    /**
     * @brief Critical icon.
     */
    CRITICAL
};

/**
 * @brief Specifies button(s) to be displayed.
 */
enum class Button {
    /**
     * @brief Display OK button only.
     */
    OK,

    /**
     * @brief Display OK and Cancel buttons.
     */
    OK_CANCEL,

    /**
     * @brief Display Yes and No buttons.
     */
    YES_NO,

    /**
     * @brief Display Yes, No and Cancel buttons.
     */
    YES_NO_CANCEL,
};

/**
 * @brief Button that the user has clicked.
 */
enum class ResultButton {
    /**
     * @brief Indicates an invalid or undefined result.
     */
    INVALID,

    /**
     * @brief Indicates the user clicked the OK button.
     */
    OK,

    /**
     * @brief Indicates the user clicked the Cancel button.
     */
    CANCEL,

    /**
     * @brief Indicates the user clicked the Yes button.
     */
    YES,

    /**
     * @brief Indicates the user clicked the No button.
     */
    NO,
};

/**
 * @brief Displays a message box, blocking the caller thread until the user dismisses the message.
 * @param parent the window the message box relates to. The OS might fade out the parent window and/or place the message
 * box within its geometry. Can be `nullptr`.
 * @param title title of the message box.
 * @param message content of the message box.
 * @param icon icon style for the message box. Defaults to NONE.
 * @param b buttons to be displayed in the message box.
 */
API_AUI_VIEWS ResultButton
show(AWindow* parent, const AString& title, const AString& message, Icon icon = Icon::NONE, Button b = Button::OK);
};   // namespace AMessageBox
