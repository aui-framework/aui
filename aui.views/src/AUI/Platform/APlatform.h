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

#include <AUI/Views.h>

class AString;
class AWindow;
namespace AMessageBox {
enum class Icon;
enum class Button;
enum class ResultButton;
}

/**
 * @brief System-specific functions.
 */
class API_AUI_VIEWS APlatform {
private:
    static std::unique_ptr<APlatform>& currentImpl();
public:
    static void init(std::unique_ptr<APlatform>&& platform);
    static APlatform& current();

    static AString getFontPath(const AString& font);

    /**
     * @brief Opens a URL using the system's default application.
     * @param url The URL to be opened, represented by an AUrl object.
     * @details
     * Attempts to open the specified URL in the user's default web browser or appropriate
     * application based on the URL schema. If opening the URL fails, an error message is logged with
     * details about the failure.
     */
    static void openUrl(const AUrl& url);

    virtual ~APlatform() = default;

    virtual void setClipboardText(const AString& text) = 0;
    virtual AString getClipboardText() = 0;

    virtual AMessageBox::ResultButton messageBoxShow(
        AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon,
        AMessageBox::Button b) = 0;
};
