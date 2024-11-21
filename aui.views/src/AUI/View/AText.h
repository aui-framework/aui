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


#include "AAbstractTextView.h"

/**
 * @brief HTML-capable text container.
 * @ingroup useful_views
 * @details
 * Used to display rich text or large texts.
 *
 * Unlike ALabel, AText is optimized to store, render, word break large texts.
 *
 * AText always prefers expanding in horizontal over vertical.
 *
 * To perform word breaking, AText requires it's width to be fully defined (either by FixedSize, Expanding or MaxSize),
 * otherwise it would require large minimum width to fit all its children in a single row. By default AText's Expanding
 * is (1, 0) (grow in width, keep minimum height). This behaviour is similar to AScrollArea.
 */
class API_AUI_VIEWS AText: public AAbstractTextView {
public:
    using Flags = AVector<std::variant<WordBreak>>;
    struct ParsedFlags {
        WordBreak wordBreak = WordBreak::NORMAL;
    };

public:
    AText();
    ~AText() override;

    void setItems(const AVector<std::variant<AString, _<AView>>>& init, const Flags& flags = {}) {
        AAbstractTextView::setItems(init, flags);
    }
    void clearContent() {
        AAbstractTextView::clearContent();
    }
    void setHtml(const AString& html, const Flags& flags = {}) {
        AAbstractTextView::setHtml(html, flags);
    }
    void setString(const AString& string, const Flags& flags) {
        AAbstractTextView::setString(string, flags);
    }
    void setString(const AString& string) {
        setString(string, {});
    }

    static _<AText> fromItems(std::initializer_list<std::variant<AString, _<AView>>> init, const Flags& flags = {}) {
        auto v = aui::ptr::manage(new AText());
        v->setItems(init, flags);
        return v;
    }
    static _<AText> fromHtml(const AString& html, const Flags& flags = {}) {
        auto v = aui::ptr::manage(new AText());
        v->setHtml(html, flags);
        return v;
    }
    static _<AText> fromString(const AString& string, const Flags& flags = {}) {
        auto v = aui::ptr::manage(new AText());
        v->setString(string, flags);
        return v;
    }

};


