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

    }
    return *mCompiledText;
}

void ATextArea::typeableErase(size_t begin, size_t end) {

}

bool ATextArea::typeableInsert(size_t at, const AString& toInsert) {
    return false;
}

ATextArea::Iterator ATextArea::getLeftEntity(size_t& index) {
    for (auto it = mEngine.entries().begin(); it != mEngine.entries().end(); ++it) {
        auto characterCount = (*it)->getCharacterCount();
        if (index > characterCount) {
            index -= characterCount;
            continue;
        }
        return it;
    }
    if (!mEngine.entries().empty()) {
        return std::prev(mEngine.entries().end());
    }
    return mEngine.entries().end();
}
bool ATextArea::typeableInsert(size_t at, char16_t toInsert) {
    auto target = getLeftEntity(at);
    if (target == mEngine.entries().end()) {
        // empty ATextArea; this one going to be the first word
        pushWord(mEngine.entries(), AString(1, toInsert));
        return true;
    }

    if (auto word = _cast<WordEntry>(*target)) {
        if (toInsert == ' ') {
            return false;
        }
        if (toInsert == '\n') {
            return false;
        }
        word->getWord().insert(at, toInsert);
        return true;
    }
    return false;
}

size_t ATextArea::typeableFind(char16_t c, size_t startPos) {
    size_t posRelativeToEntity = startPos;

    for (auto it = getLeftEntity(posRelativeToEntity); it != mEngine.entries().end(); startPos += (*it)->getCharacterCount(), ++it) {
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
            if (auto index = w->getWord().find(c, posRelativeToEntity); index != AString::NPOS) {
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
    auto target = getLeftEntity(index);
    if (target == mEngine.entries().end()) {
        return {0, 0};
    }
    if (index <= (*target)->getCharacterCount()) {
        if (auto word = _cast<WordEntry>(*target)) {
            auto base = word->getPosition();
            return base + glm::ivec2{getFontStyle().getWidth(word->getWord().begin(), word->getWord().begin() + index), 0} - mPadding.leftTop();
        }
//        return base + (*target)->getSize() - mPadding.leftTop();
    }
    auto next = std::next(target);
    if (next != mEngine.entries().end()) {
//        return (*next)->getPosition() - mPadding.leftTop();
    }
//    return (*target)->getPosition() + (*target)->getSize() - mPadding.leftTop();
    return {};
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

void ATextArea::pushWord(Entries& entries, AString word) {

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

