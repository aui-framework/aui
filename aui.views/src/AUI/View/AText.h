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


#include <AUI/Util/AWordWrappingEngine.h>
#include "AViewContainer.h"
#include <initializer_list>
#include <variant>
#include <AUI/Enum/WordBreak.h>


/**
 * @brief HTML-capable text container.
 * @ingroup useful_views
 * @details
 * Used to display rich text or large texts.
 *
 * Unlike ALabel, AText is optimized to store, render, word break large texts.
 */
class API_AUI_VIEWS AText: public AViewContainer {
public:
    using Flags = AVector<std::variant<WordBreak>>;
    struct ParsedFlags {
        WordBreak wordBreak = WordBreak::NORMAL;
    };

public:
    AText() {}

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

    void setItems(const AVector<std::variant<AString, _<AView>>>& init, const Flags& flags = {});
    void clearContent();
    void setHtml(const AString& html, const Flags& flags = {});
    void setString(const AString& string, const Flags& flags = {});

    void render(ARenderContext context) override;
    void setSize(glm::ivec2 size) override;
    int getContentMinimumWidth(ALayoutDirection layout) override;
    int getContentMinimumHeight(ALayoutDirection layout) override;
    void prerenderString(ARenderContext ctx);

    void invalidateFont() override;

    void updateLayout() override;


private:
    class CharEntry: public AWordWrappingEngine::Entry {
    private:
        AText* mText;
        char32_t mChar;
        glm::ivec2 mPosition;

    public:
        CharEntry(AText* text, char32_t ch)
                : mText(text), mChar(ch) {}

        glm::ivec2 getSize() override;

        void setPosition(const glm::ivec2& position) override;

        Float getFloat() const override;

        const glm::ivec2& getPosition() const {
            return mPosition;
        }

        char32_t getChar() const {
            return mChar;
        }
    };
    class WordEntry: public AWordWrappingEngine::Entry {
    private:
        AText* mText;
        AString mWord;
        glm::ivec2 mPosition;

    public:
        WordEntry(AText* text, AString word)
                : mText(text), mWord(std::move(word)){}

        glm::ivec2 getSize() override;

        void setPosition(const glm::ivec2& position) override;

        Float getFloat() const override;

        const glm::ivec2& getPosition() const {
            return mPosition;
        }


        const AString& getWord() const {
            return mWord;
        }
    };

    class WhitespaceEntry: public AWordWrappingEngine::Entry {
    private:
        AText* mText;

    public:
        WhitespaceEntry(AText* text) : mText(text) {}

        glm::ivec2 getSize() override;
        void setPosition(const glm::ivec2& position) override;
        Float getFloat() const override;

        bool escapesEdges() override;

        ~WhitespaceEntry() override = default;
    } mWhitespaceEntry = this;

    AWordWrappingEngine mEngine;
    ADeque<WordEntry> mWordEntries;
    ADeque<CharEntry> mCharEntries;

    ARender::PrerenderedString mPrerenderedString;
    ParsedFlags mParsedFlags;


    void pushWord(AVector<_<AWordWrappingEngine::Entry>>& entries,
                  const AString& word,
                  const ParsedFlags& flags);

    static ParsedFlags parseFlags(const Flags& flags);
};


