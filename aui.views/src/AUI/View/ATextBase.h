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


#include <AUI/Util/AWordWrappingEngine.h>
#include "AViewContainer.h"
#include "AUI/Font/IFontView.h"
#include <initializer_list>
#include <limits>
#include <variant>
#include <AUI/Enum/WordBreak.h>
#include <AUI/Enum/VerticalAlign.h>

namespace aui::detail {
    class TextBaseEntry: public AWordWrappingEngineBase::Entry {
    public:
        virtual size_t getCharacterCount() = 0;
        virtual glm::ivec2 getPosByIndex(size_t characterIndex) = 0;
        virtual void appendTo(std::u32string& dst) = 0;
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
            return { mText->getFontStyle().getCharacter(mChar).horizontal.advance, mText->getFontStyle().size };
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
            return mPosition + glm::ivec2{characterIndex * mText->getFontStyle().getCharacter(mChar).horizontal.advance, 0};
        }

        void appendTo(std::u32string& dst) override {
            dst += mChar;
        }
    };
    class WordEntry: public TextBaseEntry {
    protected:
        IFontView* mText;
        std::u32string mWord;
        glm::ivec2 mPosition{};

    public:
        WordEntry(IFontView* text, std::u32string word)
                : mText(text), mWord(std::move(word)) {}

        WordEntry(IFontView* text, AString word)
                : mText(text), mWord() {
            mWord = word.toUtf32();
        }

        glm::ivec2 getSize() override {
            return { mText->getFontStyle().getWidth(mWord), mText->getFontStyle().size };
        }

        const std::u32string& getWord() const {
            return mWord;
        }

        std::u32string& getWord() {
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
            return mPosition + glm::ivec2{mText->getFontStyle().getWidth(mWord.begin(), mWord.begin() + characterIndex), 0};
        }

        void appendTo(std::u32string& dst) override {
            dst += mWord;
        }

        void erase(size_t begin, AOptional<size_t> end) override {
            mWord.erase(mWord.begin() + begin, mWord.begin() + end.valueOr(mWord.length()));
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

        void appendTo(std::u32string& dst) override {
            dst += U' ';
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

        void appendTo(std::u32string& dst) override {
            dst += U'\n';
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

    void setVerticalAlign(VerticalAlign verticalAlign) {
        if (mVerticalAlign == verticalAlign) {
            return;
        }
        mVerticalAlign = verticalAlign;
        invalidateFont();
    }

    void doDrawString(ARenderContext& context) {
        if (!mPrerenderedString) {
            prerenderString(context);
        }
        if (mPrerenderedString) {
            RenderHints::PushColor c(context.render);
            context.render.setColor(textColor());
            mPrerenderedString->draw();
        }
    }

    void setSize(glm::ivec2 size) override {
        bool widthDiffers = size.x != getWidth();
        AViewContainerBase::setSize(size);
        if (widthDiffers) {
            mPrerenderedString = nullptr;
            requestLayout();
        }
    }

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        int max = 0;
        int accumulator = 0;
        for (const auto& e : mEngine.entries()) {
            if (e->forcesNextLine()) {
                max = glm::max(max, accumulator);
                accumulator = 0;
                continue;
            }
            accumulator += e->getSize().x;
        }
        const int preferredWidth = glm::max(max, accumulator);

        const int width = constraints.isUnlimitedWidth()
            ? glm::max(preferredWidth, constraints.minWidth)
            : glm::clamp(preferredWidth, constraints.minWidth, constraints.maxWidth);

        int height = 0;
        if (auto engineHeight = measureLayoutForWidth(width)) {
            height = *engineHeight + getFontStyle().getDescenderHeight();
        }
        return { width, height };
    }

    AMinMaxSizes onComputeIntrinsicMinMaxSizes(int widthConstraint) override {
        int max = 0;
        int accumulator = 0;
        for (const auto& e : mEngine.entries()) {
            if (e->forcesNextLine()) {
                max = glm::max(max, accumulator);
                accumulator = 0;
                continue;
            }
            accumulator += e->getSize().x;
        }
        const int preferredWidth = glm::max(max, accumulator);
        int preferredHeight = 0;
        if (auto engineHeight = measureLayoutForWidth(preferredWidth)) {
            preferredHeight = *engineHeight + getFontStyle().getDescenderHeight();
        }
        return {
            .min = { preferredWidth, preferredHeight },
            .max = { preferredWidth, preferredHeight },
        };
    }

    void invalidateFont() override {
        mPrerenderedString.reset();
        requestLayout();
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
        mPrerenderedString = nullptr;
    }

    void requestLayout() override {
        AViewContainerBase::requestLayout();
        mPrerenderedString = nullptr;
    }

protected:
    WordWrappingEngine mEngine;
    VerticalAlign mVerticalAlign = VerticalAlign::DEFAULT;

    _<IRenderer::IPrerenderedString> mPrerenderedString;


    void performLayout() {
        performLayoutForWidth(getSize().x - mPadding.horizontal());
    }

    AOptional<int> measureLayoutForWidth(int width) {
        APerformanceSection s("ATextBase::measureLayoutForWidth");
        mEngine.setTextAlign(getFontStyle().align);
        mEngine.setLineHeight(getFontStyle().lineSpacing);
        mEngine.performLayout({ 0, 0 }, { std::max(0, width), std::numeric_limits<int>::max() / 4 }, false);
        return mEngine.height();
    }

    void performLayoutForWidth(int width) {
        APerformanceSection s("ATextBase::performLayout");
        mEngine.setTextAlign(getFontStyle().align);
        mEngine.setLineHeight(getFontStyle().lineSpacing);
        mEngine.performLayout(
            {mPadding.left, mPadding.top },
            {std::max(0, width), std::numeric_limits<int>::max() / 4});
    }
};
