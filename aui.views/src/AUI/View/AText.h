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


#include "ATextBase.h"

/**
 * @brief HTML-capable text container.
 *
 * ![](imgs/views/AText.png)
 *
 * @ingroup views_indication
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
 *
 * <!-- aui:snippet aui.views/src/AUI/Devtools/DevtoolsProfilingOptions.cpp fromItems -->
 */
class API_AUI_VIEWS AText : public ATextBase<AWordWrappingEngine<>> {
public:
    using Flags = AVector<std::variant<WordBreak>>;
    struct ParsedFlags {
        WordBreak wordBreak = WordBreak::NORMAL;
    };


public:
    AText();

    ~AText() override = default;

    void clearContent() override;

    void setItems(const AVector<std::variant<AString, _<AView>>>& init, const Flags& flags = {});

    void setHtml(const AString& html, const Flags& flags = {});

    void setString(const AString& string, const Flags& flags = {});

    void setString(const AString& string) {
        setString(string, {});
    }

    static _<AText> fromItems(std::initializer_list<std::variant<AString, _<AView>>> init, const Flags& flags = {}) {
        auto v = aui::ptr::manage_shared(new AText());
        v->setItems(init, flags);
        return v;
    }

    static _<AText> fromHtml(const AString& html, const Flags& flags = {}) {
        auto v = aui::ptr::manage_shared(new AText());
        v->setHtml(html, flags);
        return v;
    }

    static _<AText> fromString(const AString& string, const Flags& flags = {}) {
        auto v = aui::ptr::manage_shared(new AText());
        v->setString(string, flags);
        return v;
    }


protected:
    void fillStringCanvas(const _<IRenderer::IMultiStringCanvas>& canvas) override;
    void applyGeometryToChildren() override;

private:
    class WordEntry final : public aui::detail::WordEntry {
    public:
        using aui::detail::WordEntry::WordEntry;
    };

    class CharEntry final : public aui::detail::CharEntry {
    public:
        using aui::detail::CharEntry::CharEntry;
    };
    _<AViewContainer> mViewsContainer;

    ADeque<WordEntry> mWordEntries;
    ADeque<CharEntry> mCharEntries;
    ParsedFlags mParsedFlags;
    aui::detail::WhitespaceEntry mWhitespaceEntry = this;
    aui::detail::NextLineEntry mNextLineEntry = this;

    void pushWord(Entries& entries,
                  AString word,
                  const ParsedFlags& flags);

    static ParsedFlags parseFlags(const Flags& flags);

    void processString(const AString& string, const ParsedFlags& parsedFlags,
                       Entries& entries);
};
