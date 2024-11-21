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
#include "AAbstractTextView.h"
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


void AAbstractTextView::pushWord(AVector<_<AWordWrappingEngine::Entry>>& entries,
                     AString word,
                     const ParsedFlags& flags) {
    if (flags.wordBreak == WordBreak::NORMAL) {
        mWordEntries.emplace_back(this, std::move(word));
        entries << aui::ptr::fake(&mWordEntries.last());
    } else {
        for (const auto& c : word) {
            mCharEntries.emplace_back(this, c);
            entries << aui::ptr::fake(&mCharEntries.last());
        }
    }
}

AAbstractTextView::ParsedFlags AAbstractTextView::parseFlags(const AAbstractTextView::Flags& flags) {
    ParsedFlags pf;
    for (auto& flag : flags) {
        std::visit(aui::lambda_overloaded {
            [&](WordBreak w) { pf.wordBreak = w; }
        }, flag);
    }
    return pf;
}

void AAbstractTextView::setString(const AString& string, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    mParsedFlags = parsedFlags;
    AVector<_<AWordWrappingEngine::Entry>> entries;
    entries.reserve(string.length());
    processString(string, parsedFlags, entries);

    mEngine.setEntries(std::move(entries));
}

void AAbstractTextView::processString(const AString& string, const AAbstractTextView::ParsedFlags& parsedFlags,
                                      AVector<_<AWordWrappingEngine::Entry>>& entries) {
    AString currentWord;
    auto commitWord = [&] {
        if (!currentWord.empty()) {
            pushWord(entries, std::move(currentWord), parsedFlags);
        }
    };
    for (auto c : string) {
        switch (c) {
            case '\r': break;
            case ' ':
                commitWord();
                entries << aui::ptr::fake(&mWhitespaceEntry);
                break;
            case '\n':
                commitWord();
                entries << aui::ptr::fake(&mNextLineEntry);
                break;

            default:
                currentWord += c;
                continue;
        }
    }
    commitWord();
}


void AAbstractTextView::setItems(const AVector<std::variant<AString, _<AView>>>& init, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    mParsedFlags = parsedFlags;
    AVector<_<AWordWrappingEngine::Entry>> entries;
    entries.reserve(init.size());
    for (auto& item : init) {
        std::visit(aui::lambda_overloaded {
            [&](const AString& string) {
                processString(string, parsedFlags, entries);
            },
            [&](const _<AView>& view) {
                addView(view);
                entries << _new<AViewEntry>(view);
            },
        }, item);
    }

    mEngine.setEntries(std::move(entries));
}

void AAbstractTextView::setHtml(const AString& html, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    AStringStream stringStream(html);
    struct CommonEntityVisitor: IXmlDocumentVisitor {
        AAbstractTextView& text;
        AVector<_<AWordWrappingEngine::Entry>> entries;
        ParsedFlags& parsedFlags;

        struct State {
            AFontStyle fontStyle;
            bool bold = false;
            bool italic = false;
        } currentState;
        std::stack<State> stateStack;

        CommonEntityVisitor(AAbstractTextView& text, ParsedFlags& parsedFlags) : text(text), parsedFlags(parsedFlags) {}

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

int AAbstractTextView::getContentMinimumWidth(ALayoutDirection layout) {
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

int AAbstractTextView::getContentMinimumHeight(ALayoutDirection layout) {
    if (getAssNames().contains("DevtoolsTest")) {
        printf("\n");
    }
    if (!mPrerenderedString) {
        performLayout();
    }

    if (auto engineHeight = mEngine.height()) {
        return *engineHeight;
    }

    return 0;
}

void AAbstractTextView::render(ARenderContext context) {
    AViewContainer::render(context);

    if (!mPrerenderedString) {
        prerenderString(context);
    }
    if (mPrerenderedString) {
        RenderHints::PushColor c(context.render);
        context.render.setColor(getTextColor());
        mPrerenderedString->draw();
    }
}

void AAbstractTextView::prerenderString(ARenderContext ctx) {
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

void AAbstractTextView::performLayout() {
    mEngine.setTextAlign(getFontStyle().align);
    mEngine.setLineHeight(getFontStyle().lineSpacing);
    mEngine.performLayout({mPadding.left, mPadding.top }, getSize() - glm::ivec2{mPadding.horizontal(), mPadding.vertical()});
}

void AAbstractTextView::setSize(glm::ivec2 size) {
    bool widthDiffers = size.x != getWidth();
    AViewContainer::setSize(size);
    if (widthDiffers) {
        mPrerenderedString = nullptr;
        markMinContentSizeInvalid();
    }
}

void AAbstractTextView::clearContent() {
    mWordEntries.clear();
    mCharEntries.clear();
    removeAllViews();
    mPrerenderedString = nullptr;
}

void AAbstractTextView::invalidateAllStyles() {
    invalidateAllStylesFont();
    AViewContainer::invalidateAllStyles();
}

void AAbstractTextView::commitStyle() {
    AView::commitStyle();
    commitStyleFont();
}

void AAbstractTextView::invalidateFont() {
    mPrerenderedString.reset();
    markMinContentSizeInvalid();
}

glm::ivec2 AAbstractTextView::NextLineEntry::getSize() {
    return {0, mText->getFontStyle().size};
}

bool AAbstractTextView::NextLineEntry::forcesNextLine() const {
    return true;
}

glm::ivec2 AAbstractTextView::WordEntry::getSize() {
    return { mText->getFontStyle().getWidth(mWord), mText->getFontStyle().size };
}

size_t AAbstractTextView::WordEntry::getCharacterCount() {
    return mWord.length();
}

glm::ivec2 AAbstractTextView::WhitespaceEntry::getSize() {
    return { mText->getFontStyle().getSpaceWidth(), mText->getFontStyle().size };
}

bool AAbstractTextView::WhitespaceEntry::escapesEdges() {
    return true;
}

glm::ivec2 AAbstractTextView::CharEntry::getSize() {
    return { mText->getFontStyle().getCharacter(mChar).advanceX, mText->getFontStyle().size };
}

void AAbstractTextView::CharEntry::setPosition(glm::ivec2 position) {
    mPosition = position;
}
