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

namespace aui::detail {
    class TextBaseEntry: public AWordWrappingEngineBase::Entry {
    public:
        virtual size_t getCharacterCount() = 0;
        virtual glm::ivec2 getPosByIndex(size_t characterIndex) = 0;
        virtual void appendTo(AString& dst) = 0;
        virtual void erase(size_t begin, AOptional<size_t> end) {}

        struct StopLineScanningHint{};
        using HitTestResult = std::variant<std::nullopt_t, size_t, StopLineScanningHint>;
        virtual HitTestResult hitTest(glm::ivec2 position) {
            return std::nullopt;
        }
    };

    class CharEntry: public TextBaseEntry {
    private:
        IFontView* mText;
        char32_t mChar;
        glm::ivec2 mPosition;

    public:
        CharEntry(IFontView* text, char32_t ch)
                : mText(text), mChar(ch) {}

        glm::ivec2 getSize() override {
            return { mText->getFontStyle().getCharacter(mChar).advanceX, mText->getFontStyle().size };
        }

        void setPosition(glm::ivec2 position) override {
            mPosition = position;
        }

        const glm::ivec2& getPosition() const {
            return mPosition;
        }

        char32_t getChar() const {
            return mChar;
        }

        size_t getCharacterCount() override {
            return 1;
        }

        glm::ivec2 getPosByIndex(size_t characterIndex) override {
            return mPosition + glm::ivec2{characterIndex * mText->getFontStyle().getCharacter(mChar).advanceX, 0};
        }

        void appendTo(AString& dst) override {
            dst += mChar;
        }
    };
    class WordEntry: public TextBaseEntry {
    protected:
        IFontView* mText;
        AString mWord;
        glm::ivec2 mPosition{};

    public:
        WordEntry(IFontView* text, AString word)
                : mText(text), mWord(std::move(word)){}

        glm::ivec2 getSize() override {
            return { mText->getFontStyle().getWidth(mWord), mText->getFontStyle().size };
        }

        const AString& getWord() const {
            return mWord;
        }

        AString& getWord() {
            return mWord;
        }

        [[nodiscard]]
        glm::ivec2 getPosition() const {
            return mPosition;
        }

        void setPosition(glm::ivec2 position) override {
            TextBaseEntry::setPosition(position);
            mPosition = position;
        }

        size_t getCharacterCount() override {
            return mWord.size();
        }

        glm::ivec2 getPosByIndex(size_t characterIndex) override {
            return mPosition + glm::ivec2{mText->getFontStyle().getWidth(mWord.begin(), mWord.begin() + long(characterIndex)), 0};
        }

        void appendTo(AString& dst) override {
            dst += mWord;
        }

        void erase(size_t begin, AOptional<size_t> end) override {
            mWord.erase(mWord.begin() + long(begin), mWord.begin() + long(end.valueOr(mWord.length())));
        }
    };

    class WhitespaceEntry: public TextBaseEntry {
    private:
        IFontView* mText;

    public:
        WhitespaceEntry(IFontView* text) : mText(text) {}

        glm::ivec2 getSize() override {
            return { mText->getFontStyle().getSpaceWidth(), mText->getFontStyle().size };
        }

        bool escapesEdges() override {
            return true;
        }

        ~WhitespaceEntry() override = default;

        size_t getCharacterCount() override {
            return 1;
        }

        glm::ivec2 getPosByIndex(size_t characterIndex) override {
            throw AException("unimplemented");
        }

        void appendTo(AString& dst) override {
            dst += ' ';
        }
    };

    class NextLineEntry: public TextBaseEntry {
    private:
        IFontView* mText;

    public:
        NextLineEntry(IFontView* text) : mText(text) {}

        bool forcesNextLine() const override {
            return true;
        }

        glm::ivec2 getSize() override {
            return {0, mText->getFontStyle().size};
        }

        ~NextLineEntry() override = default;

        size_t getCharacterCount() override {
            return 1;
        }

        glm::ivec2 getPosByIndex(size_t characterIndex) override {
            throw AException("unimplemented");
        }

        void appendTo(AString& dst) override {
            dst += '\n';
        }
    };
}

/**
 * @brief Base class for AText without public APIs.
 */
template<typename WordWrappingEngine = AWordWrappingEngine<>>
class API_AUI_VIEWS ATextBase: public AViewContainerBase, public IFontView {
public:
    using Entries = typename WordWrappingEngine::Entries;

    ATextBase() = default;
    ~ATextBase() override = default;
    void render(ARenderContext context) override {
        AViewContainerBase::render(context);

        doDrawString(context);
    }

    void doDrawString(ARenderContext& context) {
        if (!mPrerenderedString) {
            prerenderString(context);
        }
        if (mPrerenderedString) {
            RenderHints::PushColor c(context.render);
            context.render.setColor(getTextColor());
            mPrerenderedString->draw();
        }
    }

    void setSize(glm::ivec2 size) override {
        bool widthDiffers = size.x != getWidth();
        AViewContainerBase::setSize(size);
        if (widthDiffers) {
            mPrerenderedString = nullptr;
            markMinContentSizeInvalid();
        }
    }

    int getContentMinimumWidth(ALayoutDirection layout) override {
        if (mExpanding.x != 0 || mFixedSize.x != 0) {
            // there's no need to calculate min size because width is defined.
            return 0;
        }

        int accumulator = 0;
        for (const auto& e : mEngine.entries()) {
            if (accumulator + e->getSize().x > mMaxSize.x) {
                if (accumulator == 0) {
                    return mMaxSize.x;
                }
                // there's no need to calculate min size further.
                return accumulator;
            }
            accumulator += e->getSize().x;
        }
        return accumulator;
    }
    int getContentMinimumHeight(ALayoutDirection layout) override {
        if (!mPrerenderedString) {
            performLayout();
        }

        if (auto engineHeight = mEngine.height()) {
            return *engineHeight;
        }

        return 0;
    }

    void invalidateFont() override {
        mPrerenderedString.reset();
        markMinContentSizeInvalid();
    }

protected:
    void commitStyle() override {
        AView::commitStyle();
        commitStyleFont();
    }

    void invalidateAllStyles() override {
        invalidateAllStylesFont();
        AViewContainerBase::invalidateAllStyles();
    }

    virtual void fillStringCanvas(const _<IRenderer::IMultiStringCanvas>& canvas) = 0;

    void prerenderString(ARenderContext ctx) {
        performLayout();
        {
            auto multiStringCanvas = ctx.render.newMultiStringCanvas(getFontStyle());
            fillStringCanvas(multiStringCanvas);
            /*
            */
            mPrerenderedString = multiStringCanvas->finalize();
        }
    }

    virtual void clearContent() {
        removeAllViews();
        mPrerenderedString = nullptr;
    }


protected:
    WordWrappingEngine mEngine;

    _<IRenderer::IPrerenderedString> mPrerenderedString;


    void performLayout() {
        APerformanceSection s("ATextBase::performLayout");
        mEngine.setTextAlign(getFontStyle().align);
        mEngine.setLineHeight(getFontStyle().lineSpacing);
        mEngine.performLayout({mPadding.left, mPadding.top }, getSize() - glm::ivec2{mPadding.horizontal(), mPadding.vertical()});
    }
};
