/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
    std::optional<AString> mFullText;

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

    void setSize(int width, int height) override {
        bool widthChanged = getWidth() != width;
        bool heightChanged = getHeight() != height;
        AView::setSize(width, height);

        if (widthChanged) {
            updateWordWrap(getText());
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
    AString getText() const override {
        if (mFullText) {
            return *mFullText;
        }
        AString s;
        s.reserve(length());

        for (auto& l : mLines) {
            s += l.text;
        }

        return s;
    }

    size_t getTextLength() const override {
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

                Render::drawRect(ASolidBrush{},
                                 { mPadding.left + absoluteCursorPos, mPadding.top },
                                 { glm::ceil(1_dp), getFontStyle().size + 3 });
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

    void typeableInsert(size_t at, const AString& toInsert) override {
        if (!mTextArea.mEditable) {
            return;
        }
        assert(0);
    }

    void typeableInsert(size_t at, wchar_t toInsert) override {
        if (!mTextArea.mEditable) {
            return;
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
                return;
            } else {
                // not found, continue searching
                at -= length;
            }
        }

        // no lines, create a new one
        mLines.push_back({AString(toInsert), {}});
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
            if (r != AString::npos) {
                return r + s;
            }
            s = absoluteEnd;
        }
        return AString::npos;
    }

    size_t typeableReverseFind(wchar_t c, size_t startPos) override {
        return AString::npos;
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

int ATextArea::getContentMinimumHeight() {
    return 80_dp;
}

void ATextArea::onMouseWheel(const glm::ivec2& pos, const glm::ivec2& delta) {
    //AViewContainer::onMouseWheel(pos, delta);
    mScrollbar->onMouseWheel({}, delta);
}

