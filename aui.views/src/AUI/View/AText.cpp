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

#include <AUI/Util/AWordWrappingEngineImpl.h>
#include "AText.h"
#include <AUI/Xml/AXml.h>
#include <AUI/IO/AStringStream.h>
#include <AUI/Util/AViewEntry.h>
#include <stack>

AText::AText() = default;

AText::~AText() = default;

void AText::pushWord(Entries& entries,
                     AString word,
                     const ParsedFlags& flags) {
    if (flags.wordBreak == WordBreak::NORMAL) {
        mWordEntries.emplace_back(this, std::move(word));
        entries << aui::ptr::fake_shared(&mWordEntries.last());
    } else {
        for (const auto& c: word) {
            mCharEntries.emplace_back(this, c);
            entries << aui::ptr::fake_shared(&mCharEntries.last());
        }
    }
}

AText::ParsedFlags AText::parseFlags(const AText::Flags& flags) {
    ParsedFlags pf;
    for (auto& flag: flags) {
        std::visit(aui::lambda_overloaded{
                [&](WordBreak w) { pf.wordBreak = w; }
        }, flag);
    }
    return pf;
}

void AText::clearContent() {
    mWordEntries.clear();
    mCharEntries.clear();
    ATextBase::clearContent();
}

void AText::setString(const AString& string, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    mParsedFlags = parsedFlags;
    Entries entries;
    entries.reserve(string.length());
    processString(string, parsedFlags, entries);

    mEngine.setEntries(std::move(entries));
}

void AText::processString(const AString& string, const AText::ParsedFlags& parsedFlags,
                          Entries& entries) {
    AString currentWord;
    auto commitWord = [&] {
        if (!currentWord.empty()) {
            pushWord(entries, std::move(currentWord), parsedFlags);
        }
    };
    for (auto c: string) {
        switch (c) {
            case U'\r':
                break;
            case U' ':
                commitWord();
                entries << aui::ptr::fake_shared(&mWhitespaceEntry);
                break;
            case U'\n':
                commitWord();
                entries << aui::ptr::fake_shared(&mNextLineEntry);
                break;

            default:
                currentWord += c;
                continue;
        }
    }
    commitWord();
}


void AText::setItems(const AVector<std::variant<AString, _<AView>>>& init, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    mParsedFlags = parsedFlags;
    Entries entries;
    entries.reserve(init.size());
    for (auto& item: init) {
        std::visit(aui::lambda_overloaded{
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

void AText::setHtml(const AString& html, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    AStringStream stringStream(html);
    struct CommonEntityVisitor : IXmlDocumentVisitor {
        AText& text;
        Entries entries;
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

            struct ViewEntityVisitor : IXmlEntityVisitor {
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
            for (auto& w: entity.split(' ')) {
                text.pushWord(entries, w, parsedFlags);
            }
        };

    } entityVisitor(*this, parsedFlags);
    AXml::read(aui::ptr::fake_shared(&stringStream), aui::ptr::fake_shared(&entityVisitor));

    mParsedFlags = parsedFlags;
    mEngine.setEntries(std::move(entityVisitor.entries));
}

void AText::fillStringCanvas(const _<IRenderer::IMultiStringCanvas>& canvas) {
    for (auto& wordEntry: mWordEntries) {
        canvas->addString(wordEntry.getPosition(), wordEntry.getWord());
    }
    AString str(1, ' ');
    for (auto& charEntry: mCharEntries) {
        auto c = charEntry.getChar();
        if (c != ' ') {
            str.first() = c;
            canvas->addString(charEntry.getPosition(), str);
        }
    }
}
