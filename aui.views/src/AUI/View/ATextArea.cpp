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

//
// Created by alex2 on 5/22/2021.
//

#include <range/v3/all.hpp>
#include "ATextArea.h"
#include "AScrollArea.h"
#include "AUI/Traits/algorithms.h"
#include "AButton.h"
#include <AUI/Util/AWordWrappingEngineImpl.h>

namespace {

    class WhitespaceEntry final : public aui::detail::TextBaseEntry {
    private:
        IFontView* mText;
        size_t mCount;
        glm::ivec2 mPosition{};

    public:
        friend class ::ATextArea;

        WhitespaceEntry(IFontView* text, size_t count) : mText(text), mCount(count) {
            AUI_ASSERT(mCount > 0);
        }

        glm::ivec2 getSize() override {
            AUI_ASSERT(mCount > 0);
            return {mText->getFontStyle().getSpaceWidth() * mCount, mText->getFontStyle().size};
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
            return mPosition + glm::ivec2{mText->getFontStyle().getSpaceWidth() * characterIndex, 0};
        }

        void appendTo(std::u32string& dst) override {
            AUI_REPEAT(mCount) {
                dst += ' ';
            }
        }

        void erase(size_t begin, AOptional<size_t> end) override {
            auto eraseCount = end.valueOr(mCount) - begin;
            AUI_ASSERT(mCount > eraseCount);
            mCount -= eraseCount;
        }

        HitTestResult hitTest(glm::ivec2 position) override {
            position -= mPosition;
            auto size = getSize();
            if (glm::all(glm::greaterThanEqual(position, glm::ivec2(0))) &&
                glm::all(glm::lessThanEqual(position, size))) {
                // click at the left or right corner
                return size_t((mCount * position.x * 2 + mText->getFontStyle().getSpaceWidth()) / size.x / 2);
            }
            return std::nullopt;
        }
    };

    class NextLineEntry final : public aui::detail::NextLineEntry {
    public:
        friend class ATextArea;

        using aui::detail::NextLineEntry::NextLineEntry;

        glm::ivec2 getPosByIndex(size_t characterIndex) override {
            return mPosition;
        }

        void setPosition(glm::ivec2 position) override {
            Entry::setPosition(position);
            mPosition = position;
        }

    private:
        glm::ivec2 mPosition{};
    };

    class WordEntry final : public aui::detail::WordEntry {
    public:
        friend class ATextArea;

        using aui::detail::WordEntry::WordEntry;

        HitTestResult hitTest(glm::ivec2 position) override {
            position -= mPosition;
            auto lineHeight = int(mText->getFontStyle().getLineHeight()) / 2;
            if (position.y < -lineHeight) {
                return StopLineScanningHint{};
            }
            if (position.y > lineHeight) {
                return std::nullopt;
            }
            for (auto it = mWord.begin(); it != mWord.end(); ++it) {
                const int characterWidth = static_cast<int>(mText->getFontStyle().getWidth(it, std::next(it)));
                if (position.x <= characterWidth) {
                    bool rightHalf = position.x > characterWidth / 2;
                    return static_cast<size_t>(std::distance(mWord.begin(), it + (rightHalf ? 1 : 0)));
                }
                position.x -= characterWidth;
            }
            return std::nullopt;
        }
    };


    constexpr auto stringToEntriesView(ATextArea* thiz) {
        return ranges::views::filter([](char32_t c) { return c != '\r'; })
               | ranges::views::chunk_by([](char32_t prev, char32_t next) {
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
               | ranges::views::transform([thiz](ranges::range auto chars) -> _unique<aui::detail::TextBaseEntry> {
            AUI_ASSERT(!chars.empty());
            if (chars.front() == ' ') {
                return std::make_unique<WhitespaceEntry>(thiz, std::distance(chars.begin(), chars.end()));
            }
            if (chars.front() == '\n') {
                return std::make_unique<NextLineEntry>(thiz);
            }
            std::u32string u32str(chars.begin(), chars.end());
            return std::make_unique<WordEntry>(thiz, AString(u32str.data(), u32str.size()));
        });
    }
}


ATextArea::ATextArea() {
    mIsMultiline = true;
}

ATextArea::ATextArea(const AString& text) :
        ATextArea() {
    setText(text);
}

void ATextArea::setText(const AString& t) {
    auto u32strBytes = t.encode(AStringEncoding::UTF32);
    std::u32string u32str(reinterpret_cast<const char32_t*>(u32strBytes.data()), u32strBytes.size() / sizeof(char32_t));
    auto entries = u32str
                   | stringToEntriesView(this)
                   | ranges::to<Entries>();
    mEngine.setEntries(std::move(entries));

    AAbstractTypeable::setText(t);
    performLayout();
    mCompiledText = t;
}

ATextArea::~ATextArea() {

}

AString ATextArea::toString() const {
    return text();
}

AString ATextArea::getText() const {
    if (!mCompiledText) {
        std::u32string compiledText;
        compiledText.reserve(length());
        for (const auto& e: entities()) {
            e->appendTo(compiledText);
        }
        mCompiledText.emplace(AString(compiledText));
    }
    return *mCompiledText;
}

std::u32string ATextArea::getDisplayText() {
    std::u32string compiledText;
    compiledText.reserve(length());
    for (const auto& e: entities()) {
        e->appendTo(compiledText);
    }
    return compiledText;
}

void ATextArea::typeableErase(size_t begin, size_t end) {
    auto resolvedBegin = getLeftEntity(begin);
    if (resolvedBegin.iterator == entities().end()) {
        return;
    }
    mCompiledText.reset();
    AUI_DEFER { performLayout(); };
    auto resolvedEnd = getLeftEntity(end - begin, resolvedBegin);
    if (resolvedBegin.iterator == resolvedEnd.iterator) [[likely]] {
        if (resolvedBegin.relativeIndex == 0 &&
            resolvedEnd.relativeIndex == (*resolvedEnd.iterator)->getCharacterCount()) [[unlikely]] {
            entities().erase(resolvedBegin.iterator);
            return;
        }
        (*resolvedBegin.iterator)->erase(resolvedBegin.relativeIndex, resolvedEnd.relativeIndex);
        return;
    }

    if (resolvedBegin.relativeIndex != 0) [[likely]] {
        (*resolvedBegin.iterator)->erase(resolvedBegin.relativeIndex, std::nullopt);
        resolvedBegin = {.iterator = std::next(resolvedBegin.iterator), .relativeIndex = 0};
    } else {
        if (resolvedBegin.relativeIndex == (*resolvedBegin.iterator)->getCharacterCount()) {
            resolvedBegin = {.iterator = std::next(resolvedBegin.iterator), .relativeIndex = 0};
        }
    }

    if (resolvedEnd.iterator != entities().end()) {
        if (resolvedEnd.relativeIndex != (*resolvedEnd.iterator)->getCharacterCount()) {
            (*resolvedEnd.iterator)->erase(0, resolvedEnd.relativeIndex);
            resolvedEnd = {.iterator = resolvedEnd.iterator, .relativeIndex = 0};
        } else {
            resolvedEnd = {.iterator = std::next(resolvedEnd.iterator), .relativeIndex = 0};
        }
    }


    entities().erase(resolvedBegin.iterator, resolvedEnd.iterator);
}

bool ATextArea::typeableInsert(size_t at, const AString& toInsert) {
    mCompiledText.reset();
    auto [target, relativeIndex] = getLeftEntity(at);
    if (target != entities().end()) {
        if ((*target)->getCharacterCount() == relativeIndex) {
            target++;
            relativeIndex = 0;
        }
        target = splitIfNecessary({ target, relativeIndex });
    }
    auto u32bytes = toInsert.encode(AStringEncoding::UTF32);
    std::u32string u32str(reinterpret_cast<const char32_t*>(u32bytes.data()), u32bytes.size() / sizeof(char32_t));
    for (auto i: u32str | stringToEntriesView(this)) {
        target = std::next(mEngine.entries().insert(target, std::move(i)));
    }

    return true;
}

ATextArea::Iterator ATextArea::splitIfNecessary(EntityQueryResult at) {
    auto& [target, relativeIndex] = at;
    if (relativeIndex > 0) {
        if (auto leftWord = _cast<WordEntry>(*target)) {
            auto rightString = std::make_unique<WordEntry>(this, leftWord->getWord().substr(relativeIndex));
            leftWord->getWord().resize(relativeIndex);
            target = entities().insert(std::next(target), std::move(rightString));
        }
        if (auto leftWord = _cast<WhitespaceEntry>(*target)) {
            auto rightString = std::make_unique<WhitespaceEntry>(this, leftWord->mCount - relativeIndex);
            leftWord->mCount = relativeIndex;
            target = entities().insert(std::next(target), std::move(rightString));
        }
    }
    return target;
}

bool ATextArea::typeableInsert(size_t at, AChar toInsert) {
    mCompiledText.reset();
    AUI_DEFER { performLayout(); };
    auto entity = getLeftEntity(at);

    auto insertImpl = [&]<aui::factory<_unique<aui::detail::TextBaseEntry>> NewEntity, typename Append>(
            NewEntity&& newEntity, Append&& append) {
        tryAgain:
        if (entity.iterator == entities().end()) {
            entities().insert(entity.iterator, newEntity());
            return;
        }
        using T = std::decay_t<decltype(*newEntity())>;
        if constexpr (!std::is_null_pointer_v<Append>) {
            if (auto c = _cast<T>(*entity.iterator)) {
                append(*c);
                return;
            }
        }
        if ((*entity.iterator)->getCharacterCount() == entity.relativeIndex) {
            entity.iterator++;
            entity.relativeIndex = 0;
            goto tryAgain;
        }

        entity.iterator = splitIfNecessary(entity);
        entities().insert(entity.iterator, newEntity());
    };

    if (toInsert == ' ') {
        insertImpl([&] { return std::make_unique<WhitespaceEntry>(this, 1); },
                   [](WhitespaceEntry& e) { e.mCount += 1; });
        return true;
    }
    if (toInsert == '\n') {
        insertImpl([&] { return std::make_unique<NextLineEntry>(this); },
                   nullptr);
        return true;
    }

    insertImpl([&] { return std::make_unique<WordEntry>(this, AString(1, toInsert)); },
               [&](WordEntry& e) { e.getWord().insert(e.getWord().begin() + entity.relativeIndex, toInsert); });
    return true;
}

size_t ATextArea::typeableFind(AChar c, size_t startPos) {
    for (auto [it, relativeIndex] = getLeftEntity(startPos);
         it != entities().end(); startPos += (*it)->getCharacterCount() - relativeIndex, ++it, relativeIndex = 0) {
        if (relativeIndex == 0) {
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
        }
        if (auto w = _cast<WordEntry>(*it)) {
            if (auto index = w->getWord().find(c, relativeIndex); index != AString::NPOS) {
                return startPos + index;
            }
        }
    }
    return AString::NPOS;
}

size_t ATextArea::typeableReverseFind(AChar c, size_t startPos) {
    for (auto [it, relativeIndex] = getLeftEntity(startPos);
         it != entities().end(); startPos -= relativeIndex, --it, relativeIndex = (*it)->getCharacterCount()) {
        if (relativeIndex == 1) {
            if (c == ' ') {
                if (_cast<WhitespaceEntry>(*it)) {
                    return startPos - relativeIndex;
                }
            }
            if (c == '\n') {
                if (_cast<NextLineEntry>(*it)) {
                    return startPos - relativeIndex;
                }
            }
        }
        if (auto w = _cast<WordEntry>(*it)) {
            if (auto index = w->getWord().rfind(c, relativeIndex); index != AString::NPOS) {
                return startPos - index;
            }
        }
        if (it == entities().begin()) {
            break;
        }
    }
    return AString::NPOS;
}

void ATextArea::onCharEntered(AChar c) {
    AAbstractTypeableView<ATextBase>::onCharEntered(c);
    enterChar(c);
    if (textChanging) emit textChanging(text());
}

size_t ATextArea::length() const {
    if (mCompiledText) {
        return mCompiledText->length();
    }
    return ranges::accumulate(entities(), size_t(0), std::plus<>{}, [](const _unique<aui::detail::TextBaseEntry>& e) {
        return e->getCharacterCount();
    });
}

unsigned int ATextArea::cursorIndexByPos(glm::ivec2 pos) {
    unsigned accumulator = 0;
    for (const auto& e: mEngine.entries()) {
        auto result = e->hitTest(pos);
        if (std::holds_alternative<aui::detail::TextBaseEntry::StopLineScanningHint>(
                result)) { // we came way below this line.
            if (accumulator > 0) {
                return accumulator - 1;
            }
            return accumulator;
        }
        if (auto offset = std::get_if<size_t>(&result)) {
            return accumulator + *offset;
        }
        accumulator += e->getCharacterCount();
    }
    return accumulator;
}

glm::ivec2 ATextArea::getPosByIndex(size_t index) {
    auto [it, relativeIndex] = getLeftEntity(index);
    if (it == entities().end()) {
        if (it == entities().begin()) {
            return {0, 0};
        }
        it = std::prev(it);
    }
    return (*it)->getPosByIndex(glm::min(relativeIndex, (*it)->getCharacterCount())) - mPadding.leftTop();
}

glm::ivec2 ATextArea::getCursorPosition() {
    return mCursorPosition;
}

void ATextArea::onCursorIndexChanged() {
    mCursorPosition = getPosByIndex(mCursorIndex);
    AUI_NULLSAFE(findScrollArea())->scrollTo(
                ARect<int>::fromTopLeftPositionAndSize(getPositionInWindow() + mCursorPosition - mPadding.leftTop(),
                                                       {1, getFontStyle().size}));
}

void ATextArea::setSize(glm::ivec2 size) {
    ATextBase::setSize(size);
    if (auto scrollArea = findScrollArea()) {
        for (const auto& scrollbar: {scrollArea->horizontalScrollbar(), scrollArea->verticalScrollbar()}) {
            if (!scrollbar) {
                continue;
            }

            mUpdatedMaxScrollSignal = connect(scrollbar->updatedMaxScroll, this, [this] {
                AObject::disconnect();
                AUI_NULLSAFE(findScrollArea())->scrollTo(ARect<int>::fromTopLeftPositionAndSize(
                            getPositionInWindow() + mCursorPosition - mPadding.leftTop(), {1, getFontStyle().size}));
            });
        }
    }
}

void ATextArea::render(ARenderContext context) {
    AViewContainerBase::render(context);
    AStaticVector<ARect<int>, 3> selectionRects;
    if (hasSelection() && hasFocus()) {
        auto s = selection();
        auto beginPos = getPosByIndex(s.begin);
        auto endPos = getPosByIndex(s.end);
        const auto LINE_HEIGHT = int(getFontStyle().size);
        endPos.y += LINE_HEIGHT;
        for (auto i: {&beginPos, &endPos}) *i += glm::ivec2{mPadding.left, mPadding.top};
        const auto LINES_OF_SELECTION = (endPos.y - beginPos.y) / LINE_HEIGHT;
        const auto LEFT_POS = mPadding.left;
        const auto RIGHT_POS = getWidth() - mPadding.right;
        switch (LINES_OF_SELECTION) {
            default:
                // .................
                // .....############ // second rect
                // ################# // first rect
                // ################# // first rect
                // ################# // first rect
                // ################# // first rect
                // ###########...... // third rect
                // .................
                selectionRects.push_back(ARect<int>{.p1 = {LEFT_POS, beginPos.y + LINE_HEIGHT},
                        .p2 = {RIGHT_POS, endPos.y - LINE_HEIGHT}});
                [[fallthrough]];
            case 2:
                // .................
                // .....############ // first rect
                // ###########...... // second rect
                // .................
                selectionRects.push_back(ARect<int>{.p1 = beginPos, .p2 = {RIGHT_POS, beginPos.y + LINE_HEIGHT}});
                selectionRects.push_back(ARect<int>{.p1 = {LEFT_POS, endPos.y - LINE_HEIGHT}, .p2 = endPos});
                break;
            case 1:
                // .................
                // ....##########... // first rect
                // .................
                selectionRects.push_back(ARect<int>::fromTopLeftPositionAndSize(beginPos,
                                                                                endPos - beginPos));
                break;
        }
    }
    drawSelectionBeforeAndAfter(context.render, selectionRects, [&] {
        doDrawString(context);
    });

    drawCursor(context.render, mCursorPosition + mPadding.leftTop());
}

bool ATextArea::capturesFocus() {
    return true;
}

ATextArea::EntityQueryResult ATextArea::getLeftEntity(size_t index) {
    return getLeftEntity(index, {.iterator = entities().begin(), .relativeIndex = 0});
}

ATextArea::EntityQueryResult ATextArea::getLeftEntity(size_t index, EntityQueryResult from) {
    index += from.relativeIndex;
    for (auto it = from.iterator; it != entities().end(); ++it) {
        auto characterCount = (*it)->getCharacterCount();
        if (index > characterCount) {
            index -= characterCount;
            continue;
        }
        return {it, index};
    }
    if (!entities().empty()) {
        return {std::prev(entities().end()), index};
    }
    return {entities().end(), 0};
}

AScrollArea* ATextArea::findScrollArea() {
    for (auto parent = getParent(); parent != nullptr; parent = parent->getParent()) {
        if (auto scrollArea = dynamic_cast<AScrollArea*>(parent)) {
            return scrollArea;
        }
    }
    return nullptr;
}

void ATextArea::fillStringCanvas(const _<IRenderer::IMultiStringCanvas>& canvas) {
    auto wordEntries = entities()
                       | ranges::views::transform([](const _unique<aui::detail::TextBaseEntry>& e) {
        return _cast<aui::detail::WordEntry>(e);
    })
                       | ranges::views::filter([](auto ptr) { return ptr != nullptr; })
                       | ranges::views::transform([](auto ptr) -> aui::detail::WordEntry& { return *ptr; });

    for (auto& wordEntry: wordEntries) {
        canvas->addString(wordEntry.getPosition(), wordEntry.getWord());
    }
}

bool ATextArea::isPasswordField() const noexcept {
    return false;
}

ATextInputType ATextArea::textInputType() const noexcept {
    return ATextInputType::MULTILINE;
}

