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
#include "AUI/Font/IFontView.h"
#include <initializer_list>
#include <variant>
#include <AUI/Enum/WordBreak.h>


/**
 * @brief Base class for AText without public APIs.
 */
class API_AUI_VIEWS AAbstractTextView: public AViewContainer, public IFontView {
public:
    using Flags = AVector<std::variant<WordBreak>>;
    struct ParsedFlags {
        WordBreak wordBreak = WordBreak::NORMAL;
    };

public:
    AAbstractTextView() {}
    void render(ARenderContext context) override;
    void setSize(glm::ivec2 size) override;
    int getContentMinimumWidth(ALayoutDirection layout) override;
    int getContentMinimumHeight(ALayoutDirection layout) override;

    void invalidateFont() override;

protected:
    void commitStyle() override;

    void invalidateAllStyles() override;

    void prerenderString(ARenderContext ctx);

    void setItems(const AVector<std::variant<AString, _<AView>>>& init, const Flags& flags = {});
    void clearContent();
    void setHtml(const AString& html, const Flags& flags = {});
    void setString(const AString& string, const Flags& flags = {});


protected:
    class CharEntry final: public AWordWrappingEngine::Entry {
    private:
        AAbstractTextView* mText;
        char32_t mChar;
        glm::ivec2 mPosition;

    public:
        CharEntry(AAbstractTextView* text, char32_t ch)
                : mText(text), mChar(ch) {}

        glm::ivec2 getSize() override;

        void setPosition(glm::ivec2 position) override;

        const glm::ivec2& getPosition() const {
            return mPosition;
        }

        char32_t getChar() const {
            return mChar;
        }
    };
    class WordEntry final: public AWordWrappingEngine::Entry {
    private:
        AAbstractTextView* mText;
        AString mWord;

    public:
        WordEntry(AAbstractTextView* text, AString word)
                : mText(text), mWord(std::move(word)){}

        glm::ivec2 getSize() override;

        const AString& getWord() const {
            return mWord;
        }

        AString& getWord() {
            return mWord;
        }

        size_t getCharacterCount() override;
    };

    class WhitespaceEntry final: public AWordWrappingEngine::Entry {
    private:
        AAbstractTextView* mText;

    public:
        WhitespaceEntry(AAbstractTextView* text) : mText(text) {}

        glm::ivec2 getSize() override;

        bool escapesEdges() override;

        ~WhitespaceEntry() override = default;
    } mWhitespaceEntry = this;

    class NextLineEntry final: public AWordWrappingEngine::Entry {
    private:
        AAbstractTextView* mText;

    public:
        NextLineEntry(AAbstractTextView* text) : mText(text) {}

        bool forcesNextLine() const override;

        glm::ivec2 getSize() override;
        ~NextLineEntry() override = default;
    } mNextLineEntry = this;

    AWordWrappingEngine mEngine;
    ADeque<WordEntry> mWordEntries;
    ADeque<CharEntry> mCharEntries;

    _<IRenderer::IPrerenderedString> mPrerenderedString;
    ParsedFlags mParsedFlags;


    void pushWord(AVector<_<AWordWrappingEngine::Entry>>& entries,
                  AString word,
                  const ParsedFlags& flags);

    static ParsedFlags parseFlags(const Flags& flags);

    void performLayout();

    void
    processString(const AString& string, const ParsedFlags& parsedFlags,
                  AVector<_<AWordWrappingEngine::Entry>>& entries);
};


