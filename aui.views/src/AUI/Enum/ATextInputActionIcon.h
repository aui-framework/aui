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

#include <AUI/Reflect/AEnumerate.h>

/**
 * @brief Controls icons representing IME text input action the user is requested to perform.
 * @ingroup views
 * @details
 * Each action configures soft keyboard to display the certain kind of operation. The visual appearance of the action
 * button might differ depending on the target platform, it's version, and the keyboard implementation (especially on
 * Android).
 *
 * It's up to developer to handle the action accordingly.
 */
enum class ATextInputActionIcon {
    /**
     * @brief There's no concrete input action. Let the OS decide which action is the most appropriate.
     * @details
     * <dl>
     *   <dt><b>Android</b></dt>
     *   <dd>Corresponds to `IME_ACTION_UNSPECIFIED`. "Done" button or "Return" key will be likely displayed.</dd>
     * </dl>
     * <dl>
     *   <dt><b>iOS</b></dt>
     *   <dd>Corresponds to `UIReturnKeyDefault`. The title displayed in the action button is "Return".</dd>
     * </dl>
     */
    DEFAULT,

    /**
     * @brief The user is done providing input to a group of inputs (in a form). Finalization behaviour should now take place.
     * @details
     * <dl>
     *   <dt><b>Android</b></dt>
     *   <dd>Corresponds to `IME_ACTION_DONE`. The OS displays completion action, e.g., "Done", checkmark, arrow.
     * </dl>
     * <dl>
     *   <dt><b>iOS</b></dt>
     *   <dd>Corresponds to `UIReturnKeyDone`. The title displayed in the action button is "Done".</dd>
     * </dl>
     */
    DONE,

    /**
     * @brief The user has entered some kind of destination, e.g., street address. The "Go" button is intended to take
     * the user to the target of the text they typed.
     * @details
     * <dl>
     *   <dt><b>Android</b></dt>
     *   <dd>Corresponds to `IME_ACTION_GO`. The OS displays a button that represents going to the target location, e.g., "Go", right-facing arrow.
     * </dl>
     * <dl>
     *   <dt><b>iOS</b></dt>
     *   <dd>Corresponds to `UIReturnKeyGo`. The title displayed in the action button is "Go".</dd>
     * </dl>
     */
    GO,

    /**
     * @brief The user has composed some kind of message and intends to send it.
     * @details
     * <dl>
     *   <dt><b>Android</b></dt>
     *   <dd>Corresponds to `IME_ACTION_SEND`. The OS displays a button that represents sending something, e.g., "Send", paper plane.
     * </dl>
     * <dl>
     *   <dt><b>iOS</b></dt>
     *   <dd>Corresponds to `UIReturnKeySend`. The title displayed in the action button is "Send".</dd>
     * </dl>
     */
    SEND,

    /**
     * @brief Execute a search query, i.e., taking the user to the results of searching for the data they provided.
     * @details
     * <dl>
     *   <dt><b>Android</b></dt>
     *   <dd>Corresponds to `IME_ACTION_SEARCH`. The OS displays a button that represents seacrhing action, e.g., "Search", magnifying glass.
     * </dl>
     * <dl>
     *   <dt><b>iOS</b></dt>
     *   <dd>Corresponds to `UIReturnKeySearch`. The title displayed in the action button is "Search".</dd>
     * </dl>
     */
    SEARCH,

    /**
     * @brief The user has done working with the current input source ands wants to move to the next one.
     * @details
     * <dl>
     *   <dt><b>Android</b></dt>
     *   <dd>Corresponds to `IME_ACTION_NEXT`. The OS displays a button that represents moving forward, e.g., "Next", right-facing arrow.
     * </dl>
     * <dl>
     *   <dt><b>iOS</b></dt>
     *   <dd>Corresponds to `UIReturnKeyNext`. The title displayed in the action button is "Next".</dd>
     * </dl>
     */
    NEXT,
};

AUI_ENUM_VALUES(ATextInputActionIcon,
                ATextInputActionIcon::DEFAULT,
                ATextInputActionIcon::DONE,
                ATextInputActionIcon::GO,
                ATextInputActionIcon::SEND,
                ATextInputActionIcon::SEARCH,
                ATextInputActionIcon::NEXT)

