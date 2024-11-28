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

//
// Created by alex2 on 5/22/2021.
//

#include <range/v3/all.hpp>
#include "ATextArea.h"
#include "AUI/Traits/algorithms.h"
#include "AButton.h"
#include <AUI/Util/AWordWrappingEngineImpl.h>

namespace {

class WhitespaceEntry final: public aui::detail::TextBaseEntry {
private:
    IFontView* mText;
    size_t mCount;
    glm::ivec2 mPosition;

public:
    friend class ::ATextArea;

    WhitespaceEntry(IFontView* text, size_t count) : mText(text), mCount(count) {
        AUI_ASSERT(mCount > 0);
    }

    glm::ivec2 getSize() override {
        AUI_ASSERT(mCount > 0);
        return { mText->getFontStyle().getSpaceWidth() * mCount, mText->getFontStyle().size };
    }

    void setPosition(glm::ivec2 position) override {
        Entry::setPosition(position);
        mPosition = position;
    }

    bool escapesEdges() override {
        return true;
    }

    ~WhitespaceEntry() override = default;

    size_t getCharacterCount() override {
        AUI_ASSERT(mCount > 0);
        return mCount;
    }

    glm::ivec2 getPosByIndex(size_t characterIndex) override {
        return mPosition + glm::ivec2{ mText->getFontStyle().getSpaceWidth() * characterIndex, 0 };
    }

    void appendTo(AString& dst) override {
        AUI_REPEAT(mCount) {
            dst += ' ';
        }
    }
};

class NextLineEntry final: public aui::detail::NextLineEntry {
public:
    friend class ATextArea;
    using aui::detail::NextLineEntry::NextLineEntry;
};

class WordEntry final: public aui::detail::WordEntry {
public:
    friend class ATextArea;
    using aui::detail::WordEntry::WordEntry;
};
}


ATextArea::ATextArea() {
    addAssName(".input-field");
}

ATextArea::ATextArea(const AString& text):
    ATextArea()
{
    setText(text);
}

void ATextArea::setText(const AString& t) {
    auto entries = t
            | ranges::view::filter([](auto c) { return c != '\r'; })
            | ranges::view::chunk_by([](char16_t prev, char16_t next) {
                if (prev == '\n' || next == '\n') {
                    return false;
                }
                if (next == ' ' && prev != ' ') {
                    return false;
                }
                if (next != ' ' && prev == ' ') {
                    return false;
                }
                return true;
            })
            | ranges::view::transform([&](ranges::range auto chars) -> _unique<aui::detail::TextBaseEntry> {
                AUI_ASSERT(!chars.empty());
                if (chars.front() == ' ') {
                    return std::make_unique<WhitespaceEntry>(this, std::distance(chars.begin(), chars.end()));
                }
                if (chars.front() == '\n') {
                    return std::make_unique<NextLineEntry>(this);
                }
                return std::make_unique<WordEntry>(this, AString(chars.begin(), chars.end()));
            })
            | ranges::to<Entries>();
    mEngine.setEntries(std::move(entries));

    AAbstractTypeable::setText(t);
    mCompiledText = t;
}

ATextArea::~ATextArea() {

}

AString ATextArea::toString() const {
    return text();
}

const AString& ATextArea::text() const {
    if (!mCompiledText) {
        AString compiledText;
        compiledText.reserve(length());
        for (const auto& e : mEngine.entries()) {
            e->appendTo(compiledText);
        }
        mCompiledText.emplace(std::move(compiledText));
    }
    return *mCompiledText;
}

void ATextArea::typeableErase(size_t begin, size_t end) {

}

bool ATextArea::typeableInsert(size_t at, const AString& toInsert) {
    return false;
}

ATextArea::EntityQueryResult ATextArea::getLeftEntity(size_t index) {
    for (auto it = mEngine.entries().begin(); it != mEngine.entries().end(); ++it) {
        auto characterCount = (*it)->getCharacterCount();
        if (index > characterCount) {
            index -= characterCount;
            continue;
        }
        return { it, index };
    }
    if (!mEngine.entries().empty()) {
        return { std::prev(mEngine.entries().end()), index };
    }
    return { mEngine.entries().end(), 0 };
}
bool ATextArea::typeableInsert(size_t at, char16_t toInsert) {
    mCompiledText.reset();
    auto[target, relativeIndex] = getLeftEntity(at);
    if (toInsert == ' ') {
        tryAgainSpace:
        if (target == mEngine.entries().end()) {
            mEngine.entries().insert(target, std::make_unique<WhitespaceEntry>(this, 1));
            return true;
        }
        if (auto word = _cast<WhitespaceEntry>(*target)) {
            word->mCount += 1;
            return true;
        }
        if ((*target)->getCharacterCount() == relativeIndex) {
            target++;
            relativeIndex = 0;
            goto tryAgainSpace;
        }

        if (auto leftWord = _cast<WordEntry>(*target)) {
            if (relativeIndex > 0) {
                auto rightString = std::make_unique<WordEntry>(this, leftWord->getWord().substr(relativeIndex));
                leftWord->getWord().resize(relativeIndex);
                target = mEngine.entries().insert(std::next(target), std::move(rightString));
            }
            mEngine.entries().insert(target, std::make_unique<WhitespaceEntry>(this, 1));
            return true;
        }
        return false;
    }
    if (toInsert == '\n') {
        return false;
    }

    tryAgainWord:
    if (target == mEngine.entries().end()) {
        mEngine.entries().insert(target, std::make_unique<WordEntry>(this, AString(1, toInsert)));
        return true;
    }

    if (auto word = _cast<WordEntry>(*target)) {
        word->getWord().insert(relativeIndex, toInsert);
        return true;
    }

    if ((*target)->getCharacterCount() == relativeIndex) {
        target++;
        relativeIndex = 0;
        goto tryAgainWord;
    }

    if (auto leftWord = _cast<WhitespaceEntry>(*target)) {
        if (relativeIndex > 0) {
            auto rightString = std::make_unique<WhitespaceEntry>(this, leftWord->mCount - relativeIndex);
            leftWord->mCount = relativeIndex;
            target = mEngine.entries().insert(std::next(target), std::move(rightString));
        }
        mEngine.entries().insert(target, std::make_unique<WordEntry>(this, AString(1, toInsert)));
        return true;
    }

    return false;
}

size_t ATextArea::typeableFind(char16_t c, size_t startPos) {
    for (auto[it, relativeIndex] = getLeftEntity(startPos); it != mEngine.entries().end(); startPos += (*it)->getCharacterCount(), ++it, relativeIndex = 0) {
        if (c == ' ') {
            if (_cast<WhitespaceEntry>(*it)) {
                return startPos;
            }
        }
        if (c == '\n') {
            if (_cast<NextLineEntry>(*it)) {
                return startPos;
            }
        }
        if (auto w = _cast<WordEntry>(*it)) {
            if (auto index = w->getWord().find(c, relativeIndex); index != AString::NPOS) {
                return startPos + index;
            }
        }
    }
    return AString::NPOS;
}

size_t ATextArea::typeableReverseFind(char16_t c, size_t startPos) {
    auto it = getLeftEntity(startPos);
    return 0;
}

void ATextArea::onCharEntered(char16_t c) {
    AAbstractTypeableView<ATextBase>::onCharEntered(c);
    enterChar(c);
    if (textChanging) emit textChanging(text());
}

size_t ATextArea::length() const {
    if (mCompiledText) {
        return mCompiledText->length();
    }
    return ranges::accumulate(mEngine.entries(), size_t(0), std::plus<>{}, [](const _unique<aui::detail::TextBaseEntry>& e) {
        return e->getCharacterCount();
    });
}

unsigned int ATextArea::cursorIndexByPos(glm::ivec2 pos) {
    return 0;
}

glm::ivec2 ATextArea::getPosByIndex(size_t index) {
    auto[it, relativeIndex] = getLeftEntity(index);
    if (it == mEngine.entries().end()) {
        return {0, 0};
    }
    return (*it)->getPosByIndex(glm::min(relativeIndex, (*it)->getCharacterCount())) - mPadding.leftTop();
}

void ATextArea::onCursorIndexChanged() {
    mCursorPosition = getPosByIndex(mCursorIndex);
}

void ATextArea::render(ARenderContext context) {
    ATextBase::render(context);
    drawCursor(context.render, mCursorPosition + mPadding.leftTop());
}

bool ATextArea::capturesFocus() {
    return true;
}

auto ATextArea::wordEntries() const {
    return mEngine.entries()
        | ranges::view::transform([](const _unique<aui::detail::TextBaseEntry>& e) {
            return _cast<aui::detail::WordEntry>(e);
        })
        | ranges::view::filter([](auto ptr) { return ptr != nullptr; })
        | ranges::view::transform([](auto ptr) -> aui::detail::WordEntry& { return *ptr; });
}

auto ATextArea::charEntries() const {
    return std::initializer_list<aui::detail::CharEntry>{};
}

