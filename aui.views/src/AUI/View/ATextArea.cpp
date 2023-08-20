// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 5/22/2021.
//


#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/AMetric.h>
#include "ATextArea.h"
#include <optional>
#include <glm/ext/matrix_transform.hpp>


class ATextArea::TextAreaField: public AAbstractTypeableView {
private:
    ATextArea& mTextArea;
    struct Line {
        AString text;
        Render::PrerenderedString prerendered;
    };

    AVector<Line> mLines;

    /**
     * Full text. Must be set to null if mLines changed.
     */
    mutable AOptional<AString> mFullText;

    int mScroll = 0;

    void updateWordWrap(const AString& text) {
        mLines.clear();
        mFullText = text;
    }

    void updateScrollDimensions() {
        mTextArea.mScrollbar->setScrollDimensions(getContentHeight(),
                                                  mLines.size() * getFontStyle().getLineHeight());
    }
    void pushScrollMatrix() {
        Render::setTransform(glm::translate(glm::mat4(), glm::vec3(0, -mScroll, 0)));
    }

public:
    TextAreaField(ATextArea& textArea) : mTextArea(textArea) {

        connect(mTextArea.mScrollbar->scrolled, [&](int scroll) {
            mScroll = scroll;
            redraw();
        });
        addAssName(".text-area-field");
    }

    void setSize(glm::ivec2 size) override {
        bool widthChanged = getWidth() != size.x;
        bool heightChanged = getHeight() != size.y;
        AView::setSize(size);

        if (widthChanged) {
            updateWordWrap(text());
        } else if (heightChanged) {
            updateScrollDimensions();
        }
    }

    void setText(const AString& t) override {
        mLines.clear();
        AAbstractTypeableView::setText(t);
        updateWordWrap(t);
    }

    /**
     * @note This function is expensive!
     * @return containing text
     */
    const AString& text() const override {
        if (!mFullText) {
            AString s;
            s.reserve(length());

            for (auto& l: mLines) {
                s += l.text;
            }
            mFullText = std::move(s);
        }
        return *mFullText;
    }

    size_t textLength() const override {
        size_t length = 0;
        for (auto& l : mLines) {
            length += l.text.length();
        }
        return length;
    }

    void render() override {
        // TODO stub
        /*
        if (mLines.empty() && !mFullText->empty()) {
            size_t wordWrappingPos = 0;
            while (wordWrappingPos < mFullText->length()) {
                AString line = getFontStyle().font->trimStringToWidth(mFullText->begin() + wordWrappingPos,
                                                                      mFullText->end(),
                                                                      getContentWidth(),
                                                                      getFontStyle().size,
                                                                      getFontStyle().fontRendering);
                wordWrappingPos += line.length() + 1;
                mLines.push_back(Line{ std::move(line), {} });
            }
            updateScrollDimensions();
        }*/
        AView::render();

        auto drawText = [&] {
            size_t lineHeight = getFontStyle().getLineHeight();
            size_t viewHeight = getContentHeight();
            for (size_t i = mScroll / getFontStyle().getLineHeight(); i < mLines.size(); ++i) {
                if (i * lineHeight > viewHeight + mScroll) {
                    return;
                }

                if (!mLines[i].prerendered) {
                    mLines[i].prerendered = Render::prerenderString({0, 0}, mLines[i].text, getFontStyle());
                }
                RenderHints::PushMatrix m;
                Render::translate({ mPadding.left - mHorizontalScroll,
                                    mPadding.top + i * getFontStyle().getLineHeight() - mScroll });
                mLines[i].prerendered->draw();
            }
        };

        if (hasFocus() && mTextArea.mEditable) {
            int absoluteCursorPos;
            {
                RenderHints::PushMatrix m;
                pushScrollMatrix();
                absoluteCursorPos = ACursorSelectable::drawSelectionPre();
            }

            drawText();

            {
                RenderHints::PushMatrix m;
                ACursorSelectable::drawSelectionPost();
            }

            // cursor
            if (mTextArea.mEditable && hasFocus() && isCursorBlinkVisible()) {
                if (absoluteCursorPos < 0) {
                    mHorizontalScroll += absoluteCursorPos;
                    redraw();
                } else if (getWidth() < absoluteCursorPos + mPadding.horizontal() + 1) {
                    mHorizontalScroll += absoluteCursorPos - getWidth() + mPadding.horizontal() + 1;
                    redraw();
                }

                Render::rect(ASolidBrush{},
                             {mPadding.left + absoluteCursorPos, mPadding.top},
                             {glm::ceil(1_dp), getFontStyle().size + 3});
            }
            // TODO STUB
            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            drawText();
        }
    }

    void onCharEntered(wchar_t c) override {
        AView::onCharEntered(c);
        if (mTextArea.mEditable) {
            enterChar(c);
        }
    }

protected:
    void invalidatePrerenderedString() override {
        for (auto& l : mLines) {
            l.prerendered = nullptr;
        }
    }

    void typeableErase(size_t begin, size_t end) override {
        if (!mTextArea.mEditable) {
            return;
        }
        // find line number and offset
        size_t lineNumber = 0;
        size_t offset = 0;
        for (size_t currentLineLength; ; offset -= currentLineLength, ++lineNumber) {
            if (lineNumber >= mLines.size()) {
                // error: out of bounds
                return;
            }
            currentLineLength = mLines[lineNumber].text.length();
            if (begin - offset < currentLineLength) {
                // found
                break;
            }
        }

        // do erase
        auto& firstLine = mLines[lineNumber];
        auto& firstText = firstLine.text;
        if (end == -1) {
            // _________
            // _____####
            // #########
            // #########

            // erase string till end
            firstText.erase(firstText.begin() + (begin - offset), firstText.end());

            // erase lines till end
            mLines.erase(mLines.begin() + lineNumber + 1, mLines.end());
            return;
        } else {
            // check if erase ends before the end of the line
            if ((begin - offset) < firstText.length()) {
                // _________
                // __###____
                // _________
                firstText.erase(firstText.begin() + begin - offset, firstText.begin() + end - offset);
            } else {
                if (begin == offset) {
                    // _________
                    // #########
                    // #########
                    // ######___

                    // we can delete whole lines
                    // this breaks firstLine and firstText
                    do {
                        offset += mLines[lineNumber].text.length();
                        mLines.erase(mLines.begin() + lineNumber);
                    } while ((end - offset) > mLines[lineNumber].text.length());

                    // the last line left
                    if (end > offset) {
                        // ######___
                        auto& t = mLines[lineNumber].text;
                        t.erase(t.begin(), t.begin() + (end - offset));
                    }
                } else {
                    // _________
                    // ___#######
                    // #########
                    // ######___
                    assert(0);
                }
            }
            firstText.erase(firstText.begin() + (begin - offset), firstText.begin() + (end - offset));
            firstLine.prerendered = nullptr;
        }
    }

    bool typeableInsert(size_t at, const AString& toInsert) override {
        if (!mTextArea.mEditable) {
            return false;
        }
        assert(0);
        return true;
    }

    bool typeableInsert(size_t at, wchar_t toInsert) override {
        if (!mTextArea.mEditable) {
            return false;
        }
        for (size_t i = 0; i < mLines.size(); ++i) {
            auto& text = mLines[i].text;
            size_t length = text.length();
            if (at <= length) {
                // found target line
                // at is now offset relative to begin of current line text

                // insert single symbol
                text.insert(at++, toInsert);
                if (toInsert == '\n') {
                    if (text.length() == at) {
                        // just create a new blank line
                        mLines.insert(mLines.begin() + at, {});
                    } else {
                        // split line
                        auto secondPart = text.substr(at);
                        text.resize(at);
                        mLines.insert(mLines.begin() + at, {secondPart, {}});
                        mLines[i].prerendered = nullptr;
                    }
                } else {
                    mLines[i].prerendered = nullptr;
                }
                return true;
            } else {
                // not found, continue searching
                at -= length;
            }
        }

        // no lines, create a new one
        mLines.push_back({AString(toInsert), {}});
        return false;
    }

    size_t typeableFind(wchar_t c, size_t startPos) override {
        size_t s = 0;
        for (auto& l : mLines) {
            size_t absoluteEnd = s + l.text.length();
            size_t r;
            if (startPos <= s) {
                // ignore startPos
                r = l.text.find(c);
            } else if (absoluteEnd < startPos) {
                r = l.text.find(c, startPos - s);
            } else {
                s = absoluteEnd;
                continue;
            }
            if (r != AString::NPOS) {
                return r + s;
            }
            s = absoluteEnd;
        }
        return AString::NPOS;
    }

    size_t typeableReverseFind(wchar_t c, size_t startPos) override {
        return AString::NPOS;
    }

    size_t length() const override {
        size_t accumulator = 0;

        for (auto& l : mLines) {
            accumulator += l.text.length();
        }

        return accumulator;
    }
};

ATextArea::ATextArea() {
    addAssName(".input-field");
    setLayout(_new<AHorizontalLayout>());
    mScrollbar = _new<AScrollbar>();
    addView(mTextField = _new<TextAreaField>(*this) let {
        it->setExpanding();
    });
    addView(mScrollbar);
}

ATextArea::ATextArea(const AString& text):
    ATextArea()
{
    mTextField->setText(text);
}

int ATextArea::getContentMinimumHeight(ALayoutDirection layout) {
    return 80_dp;
}

void ATextArea::onMouseScroll(const AScrollEvent& event) {
    //AViewContainer::onMouseScroll(pos, delta);
    mScrollbar->onMouseScroll(event);
}

