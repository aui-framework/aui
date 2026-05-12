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

#include "AText.h"

#include <AUI/Util/AWordWrappingEngineImpl.h>
#include <AUI/View/ALabel.h>
#include <AUI/IO/AByteBufferInputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Xml/AXml.h>
#include <AUI/IO/AStringStream.h>
#include <AUI/Util/AViewEntry.h>
#include <stack>

AText::AText() {
  addView(mViewsContainer = _new<AViewContainer>());
}

AMinMaxAxis AText::onComputeIntrinsicMinMaxAxis(int height) {
  auto minMax = ATextBase::onComputeIntrinsicMinMaxAxis(height);

  int minContentWidth = 0;
  for (const auto& entry : mEngine.entries()) {
    if (entry->forcesNextLine() || entry->escapesEdges()) {
      continue;
    }
    minContentWidth = glm::max(minContentWidth, entry->getSize().x);
  }

  minMax.min = minContentWidth;
  return minMax;
}

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
  mViewsContainer->removeAllViews();
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

void AText::processString(AStringView string, const AText::ParsedFlags& parsedFlags,
                          Entries& entries) {
  AString currentWord;
  auto commitWord = [&] {
    if (!currentWord.empty()) {
      pushWord(entries, std::move(currentWord), parsedFlags);
    }
  };
  for (auto c: string) {
    switch (c) {
      case '\r':
        break;
      case ' ':
        commitWord();
        entries << aui::ptr::fake_shared(&mWhitespaceEntry);
        break;
      case '\n':
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
                    mViewsContainer->addView(view);
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

void AText::setMarkdown(const AString& md, const Flags& flags) {
    clearContent();
    auto parsedFlags = parseFlags(flags);
    Entries entries;
    ATokenizer tokenizer(_new<AByteBufferInputStream>(AByteBufferView(md.data(), md.size())));
    std::string lastWord;
    bool rightAfterNewLine = true;
    bool italic = false;
    bool bold = false;
    int headerLevel = 0;

    auto handleWord = [&] {
        if (lastWord.empty()) {
            return;
        }
        auto label = _new<ALabel>(std::exchange(lastWord, {}));
        label->getFontStyle().italic = italic;
        label->getFontStyle().bold = bold;
        label->setCustomStyle({
          ass::FontSize { [&] {
              // TODO: hardcoded: need to provide a way to adjust these
              switch (headerLevel) {
                  default:
                  case 0:
                      return 10_pt;
                  case 1:
                      return 24_pt;
                  case 2:
                      return 18_pt;
                  case 3:
                      return 16_pt;
                  case 4:
                      return 14_pt;
                  case 5:
                      return 12_pt;
              }
          }() },
        });
        mViewsContainer->addView(label);
        entries << _new<AViewEntry>(std::move(label));
    };
    try {
        for (;;) {
            std::function<void()> doAfterWord; // lol dirty
            switch (auto c = tokenizer.readChar()) {
                case '*':
                    doAfterWord = [&] {
                        if (tokenizer.readChar() == '*') {
                            bold = !bold;
                        } else {
                            tokenizer.reverseByte();
                            italic = !italic;
                        }
                    };
                    break;

                case ' ':
                case '\t':
                    if (rightAfterNewLine) {
                        continue; // skip dangling whitespaces
                    }
                    doAfterWord = [&] {
                        entries << aui::ptr::fake_shared(&mWhitespaceEntry);
                        // auto dot = _new<ALabel>("\u00b7") AUI_OVERRIDE_STYLE { ass::Opacity { 0.8f} };
                        // mViewsContainer->addView(dot);
                        // entries << _new<AViewEntry>(std::move(dot));
                    };
                    break;
                case '\n':
                    doAfterWord = [&] {
                        rightAfterNewLine = true;
                        entries << aui::ptr::fake_shared(&mNextLineEntry);
                        headerLevel = 0;
                    };
                    break;

                case '#':
                    if (rightAfterNewLine) {
                        // treat this as header only if we have started from a fresh line.
                        doAfterWord = [&] {
                            headerLevel++;
                        };
                        break;
                    }
                    [[fallthrough]];

                default:
                    rightAfterNewLine = false;
                    lastWord += c;
                    continue;
            }

            handleWord();
            AUI_NULLSAFE(doAfterWord)();
        }
    } catch (const AEOFException& e) {}
    handleWord();
    mEngine.setEntries(std::move(entries));
    mParsedFlags = parsedFlags;
}

void AText::fillStringCanvas(const _<IRenderer::IMultiStringCanvas>& canvas) {
  auto ascender = glm::ivec2 {0, getFontStyle().getAscenderHeight() + getFontStyle().getDescenderHeight()};
  const int textHeight =
      this->measure(AConstraints::fixedInline(getContentWidth() + getPadding().horizontal())).y - getPadding().vertical();
  if (mVerticalAlign == VerticalAlign::MIDDLE) {
    ascender += (getContentHeight() - textHeight) / 2;
  }
  for (auto& wordEntry: mWordEntries) {
    canvas->addString(wordEntry.getPosition() + ascender, wordEntry.getWord());
  }
  for (auto& charEntry: mCharEntries) {
    auto c = charEntry.getChar();
    if (c != ' ') {
      AString str(1, c);
      canvas->addString(charEntry.getPosition() + ascender, str);
    }
  }
}
void AText::onLayout(int w, int h) {
  ATextBase::onLayout(w, h);

  int y = 0;
  const int textHeight =
      this->measure(AConstraints::fixedInline(getContentWidth() + getPadding().horizontal())).y - getPadding().vertical();
  if (mVerticalAlign == VerticalAlign::MIDDLE) {
    y += (getContentHeight() - textHeight) / 2;
  }
  mViewsContainer->layout(0, y, w, h);
}
