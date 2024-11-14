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
// Created by Alex2772 on 11/24/2021.
//

#include <AUI/Traits/callables.h>
#include <AUI/IO/AStringStream.h>
#include <AUI/Xml/AXml.h>
#include "AText.h"
#include "AButton.h"
#include <stack>
#include <AUI/Util/kAUI.h>
#include <chrono>
#include <utility>
#include <AUI/Traits/strings.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/AViewEntry.h>

struct State {
    AFontStyle fontStyle;
    bool bold = false;
    bool italic = false;
};


void AText::pushWord(AVector<_<AWordWrappingEngine::Entry>>& entries,
                     const AString& word,
                     const ParsedFlags& flags) {
    if (!entries.empty()) {
        entries << aui::ptr::fake(&mWhitespaceEntry);
    }
    if (flags.wordBreak == WordBreak::NORMAL) {
        mWordEntries.emplace_back(this, word);
        entries << aui::ptr::fake(&mWordEntries.last());
    } else {
        for (const auto& c : word) {
            mCharEntries.emplace_back(this, c);
            entries << aui::ptr::fake(&mCharEntries.last());
        }
    }
}

AText::ParsedFlags AText::parseFlags(const AText::Flags& flags) {
    ParsedFlags pf;
    for (auto& flag : flags) {
        std::visit(aui::lambda_overloaded {
            [&](WordBreak w) { pf.wordBreak = w; }
        }, flag);
    }
    return pf;
}

void AText::setString(const AString& string, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    mParsedFlags = parsedFlags;
    AVector<_<AWordWrappingEngine::Entry>> entries;
    auto splt = string.split(' ');
    entries.reserve(splt.size());
    for (auto& w : splt) {
        pushWord(entries, w, parsedFlags);
    }

    mEngine.setEntries(std::move(entries));
}


void AText::setItems(const AVector<std::variant<AString, _<AView>>>& init, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    mParsedFlags = parsedFlags;
    AVector<_<AWordWrappingEngine::Entry>> entries;
    entries.reserve(init.size());
    for (auto& item : init) {
        std::visit(aui::lambda_overloaded {
            [&](const AString& string) {
                for (auto& w : string.split(' ')) {
                    pushWord(entries, w, parsedFlags);
                }
            },
            [&](const _<AView>& view) {
                addView(view);
                entries << _new<AViewEntry>(view);
            },
        }, item);
    }

    mEngine.setEntries(std::move(entries));
}

void AText::setHtml(const AString& html, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    AStringStream stringStream(html);
    struct CommonEntityVisitor: IXmlDocumentVisitor {
        AText& text;
        AVector<_<AWordWrappingEngine::Entry>> entries;
        ParsedFlags& parsedFlags;

        struct State {
            AFontStyle fontStyle;
            bool bold = false;
            bool italic = false;
        } currentState;
        std::stack<State> stateStack;

        CommonEntityVisitor(AText& text, ParsedFlags& parsedFlags) : text(text), parsedFlags(parsedFlags) {}

        void visitAttribute(const AString& name, AString value) override {};
        _<IXmlEntityVisitor> visitEntity(AString entityName) override {

            struct ViewEntityVisitor: IXmlEntityVisitor {
                CommonEntityVisitor& parent;
                AString name;
                AMap<AString, AString> attrs;

                ViewEntityVisitor(CommonEntityVisitor& parent, AString name) : parent(parent), name(std::move(name)) {}

                void visitAttribute(const AString& attributeName, AString value) override {
                    attrs[attributeName] = std::move(value);
                }

                _<IXmlEntityVisitor> visitEntity(AString entityName) override {
                    return nullptr;
                }

                void visitTextEntity(const AString& entity) override {
                    IXmlEntityVisitor::visitTextEntity(entity);
                }

                ~ViewEntityVisitor() override {
                    auto view = _new<AButton>("hello {}"_format(name));
                    parent.text.addView(view);
                    parent.entries << _new<AViewEntry>(view);
                }
            };

            return _new<ViewEntityVisitor>(*this, std::move(entityName));
        };
        void visitTextEntity(const AString& entity) override {
            for (auto& w : entity.split(' ')) {
                text.pushWord(entries, w, parsedFlags);
            }
        };

    } entityVisitor(*this, parsedFlags);
    AXml::read(aui::ptr::fake(&stringStream), aui::ptr::fake(&entityVisitor));

    mParsedFlags = parsedFlags;
    mEngine.setEntries(std::move(entityVisitor.entries));
}

int AText::getContentMinimumWidth(ALayoutDirection layout) {
    if (!mPrerenderedString) {
        performLayout();
    }
    return mPrerenderedString ? mPrerenderedString->getWidth() : 0;
}

int AText::getContentMinimumHeight(ALayoutDirection layout) {
    if (!mPrerenderedString) {
        performLayout();
    }
    auto height = mPrerenderedString ? mPrerenderedString->getHeight() : 0;

    if (auto engineHeight = mEngine.height()) {
        height = std::max(height, *engineHeight);
    }

    return height;
}

void AText::render(ARenderContext context) {
    AViewContainer::render(context);

    if (!mPrerenderedString) {
        prerenderString(context);
    }
    if (mPrerenderedString) {
        context.render.setColor(getTextColor());
        mPrerenderedString->draw();
    }
}

void AText::prerenderString(ARenderContext ctx) {
    performLayout();
    {
        auto multiStringCanvas = ctx.render.newMultiStringCanvas(getFontStyle());

        if (mParsedFlags.wordBreak == WordBreak::NORMAL) {
            for (auto& wordEntry: mWordEntries) {
                multiStringCanvas->addString(wordEntry.getPosition(), wordEntry.getWord());
            }
        } else {
            AString str(1, ' ');
            for (auto& charEntry: mCharEntries) {
                auto c = charEntry.getChar();
                if (c != ' ') {
                    str.first() = c;
                    multiStringCanvas->addString(charEntry.getPosition(), str);
                }
            }
        }
        mPrerenderedString = multiStringCanvas->finalize();
    }
}

void AText::performLayout() {
    mEngine.setTextAlign(getFontStyle().align);
    mEngine.setLineHeight(getFontStyle().lineSpacing);
    mEngine.performLayout({mPadding.left, mPadding.top }, getSize());
}

void AText::setSize(glm::ivec2 size) {
    bool widthDiffers = size.x != getWidth();
    int prevContentMinimumHeight = getContentMinimumHeight(ALayoutDirection::NONE);
    AViewContainer::setSize(size);
    if (widthDiffers) {
        mPrerenderedString = nullptr;
        markMinContentSizeInvalid();
    }
}

glm::ivec2 AText::WordEntry::getSize() {
    return { mText->getFontStyle().getWidth(mWord), mText->getFontStyle().size };
}

void AText::WordEntry::setPosition(const glm::ivec2& position) {
    mPosition = position;
}

Float AText::WordEntry::getFloat() const {
    return Float::NONE;
}

glm::ivec2 AText::WhitespaceEntry::getSize() {
    return { mText->getFontStyle().getSpaceWidth(), mText->getFontStyle().size };
}

void AText::WhitespaceEntry::setPosition(const glm::ivec2& position) {

}

Float AText::WhitespaceEntry::getFloat() const {
    return Float::NONE;
}

bool AText::WhitespaceEntry::escapesEdges() {
    return true;
}


glm::ivec2 AText::CharEntry::getSize() {
    return { mText->getFontStyle().getCharacter(mChar).advanceX, mText->getFontStyle().size };
}

void AText::CharEntry::setPosition(const glm::ivec2& position) {
    mPosition = position;
}

Float AText::CharEntry::getFloat() const {
    return Float::NONE;
}

void AText::clearContent() {
    mWordEntries.clear();
    mCharEntries.clear();
    removeAllViews();
    mPrerenderedString = nullptr;
}

void AText::invalidateAllStyles() {
    invalidateAllStylesFont();
    AViewContainer::invalidateAllStyles();
}

void AText::commitStyle() {
    AView::commitStyle();
    commitStyleFont();
}

void AText::invalidateFont() {
    mPrerenderedString.reset();
    markMinContentSizeInvalid();
}
