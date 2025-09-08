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
// Created by alex2 on 5/23/2021.
//


#include <AUI/Platform/AClipboard.h>
#include "AAbstractTypeable.h"


#include "AUI/Platform/APlatform.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/Util/ARaiiHelper.h"
#include <AUI/Util/AMetric.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/kAUI.h>
#include <AUI/i18n/AI18n.h>

_<ATimer> AAbstractTypeable::blinkTimer()
{
    using namespace std::chrono_literals;
    static _weak<ATimer> t;
    if (auto l = t.lock()) {
        return l;
    }
    auto timer = _new<ATimer>(500ms);
    timer->start();
    t = timer;
    return timer;
}

void AAbstractTypeable::updateCursorBlinking()
{
    mBlinkTimer->restart();
    mCursorBlinkVisible = true;
    mCursorBlinkCount = 0;
    cursorSelectableRedraw();
}

AAbstractTypeable::AAbstractTypeable()
{

}

AAbstractTypeable::~AAbstractTypeable()
{
}

void AAbstractTypeable::handleKey(AInput::Key key)
{
    if (AInput::isKeyDown(AInput::LBUTTON))
        return;

    mTextChangedFlag = true;
    switch (key)
    {
        case AInput::BACKSPACE:
            if (hasSelection()) {
                eraseSelection();
            } else {
                if (mCursorIndex != 0) {
                    typeableErase(mCursorIndex - 1, mCursorIndex);
                    mCursorIndex -= 1;
                    typeableInvalidateFont();
                }
            }
            break;
        case AInput::DEL:
            if (hasSelection()) {
                eraseSelection();
            } else {
                if (AInput::isKeyDown(AInput::LCONTROL) || AInput::isKeyDown(AInput::RCONTROL)) {
                    auto index = typeableFind(' ', mCursorIndex);
                    if (index == AString::NPOS) {
                        index = length();
                    } else {
                        index = index + 1;
                    }

                    typeableErase(mCursorIndex, index);
                    typeableInvalidateFont();
                } else {
                    if (mCursorIndex < length())
                    {
                        typeableErase(mCursorIndex, mCursorIndex + 1);
                        typeableInvalidateFont();
                    }
                }
            }
            break;

        case AInput::LEFT:
            moveCursorLeft();
            return;

        case AInput::RIGHT:
            moveCursorRight();
            return;

        case AInput::HOME:
            fastenSelection();
            mCursorIndex = 0;
            break;
        case AInput::END:
            fastenSelection();
            mCursorIndex = length();
            break;

        default:
            if (AInput::isKeyDown(AInput::LCONTROL) || AInput::isKeyDown(AInput::RCONTROL)) {
                switch (key) {
                    case AInput::A: // select all
                        selectAll();
                        break;

                    case AInput::C: // copy
                        copyToClipboard();
                        break;

                    case AInput::X: // cut
                        cutToClipboard();
                        break;

                    case AInput::V: // paste
                        pasteFromClipboard();
                        break;

                    default:
                        return;
                }
            } else {
                return;
            }
    }

    if (textChanging) {
        emitTextChanging(getText());
    }

    onCursorIndexChanged();
    updateCursorBlinking();

    cursorSelectableRedraw();
}

void AAbstractTypeable::fastenSelection() {
    if (!AInput::isKeyDown(AInput::LSHIFT) && !AInput::isKeyDown(AInput::RSHIFT)) {
        mCursorSelection.reset();
    } else if (!mCursorSelection) {
        mCursorSelection = mCursorIndex;
    }
}

void AAbstractTypeable::eraseSelection() {
    AUI_ASSERT(hasSelection());

    auto sel = selection();
    typeableErase(sel.begin, sel.end);
    typeableInvalidateFont();
    mCursorSelection.reset();
    mCursorIndex = sel.begin;
}

void AAbstractTypeable::paste(AString content) {
    auto pastePos = mCursorIndex;
    AOptional<AString> prevContents;
    if (mCursorSelection) {
        prevContents = getText();
        auto sel = selection();
        pastePos = sel.begin;
        typeableErase(sel.begin, sel.end);
    }
    if (mMaxTextLength <= length())
        return;

    if (!mIsMultiline) {
        content = content.replacedAll("\n", "");
    }
    if (typeableInsert(pastePos, content)) {
        mCursorIndex = pastePos + content.length();
        mCursorSelection.reset();

        typeableInvalidateFont();
        onCursorIndexChanged();
    } else if (prevContents) {
        setText(*prevContents);
    }
}

void AAbstractTypeable::pasteFromClipboard() {
    paste(AClipboard::pasteFromClipboard());
}

void AAbstractTypeable::cutToClipboard() {
    if (!mIsCopyable)
        return;

    auto sel = selection();
    AClipboard::copyToClipboard(selectedText());
    typeableErase(sel.begin, sel.end);
    mCursorIndex = sel.begin;
    mCursorSelection.reset();
    typeableInvalidateFont();
    onCursorIndexChanged();
}

void AAbstractTypeable::copyToClipboard() const {
    if (!mIsCopyable)
        return;

    AClipboard::copyToClipboard(selectedText());
}

void AAbstractTypeable::enterChar(AChar c)
{
    if (AInput::isKeyDown(AInput::LBUTTON) ||
        AInput::isKeyDown(AInput::LCONTROL) ||
        AInput::isKeyDown(AInput::RCONTROL) ||
        c == '\t')
        return;
    if (c == '\b') { // some OSes report backspace as char event.
        return;
    }
    if (c == '\r') {
        c = '\n';
    }

    mTextChangedFlag = true;
    auto cursorIndexCopy = mCursorIndex;

    if (hasSelection()) {
        auto sel = selection();
        typeableErase(sel.begin, sel.end);

        switch (c)
        {
            default:
                if (typeableInsert(sel.begin, c)) {
                    mCursorIndex = sel.begin + 1;
                } else {
                    mCursorIndex = sel.begin;
                }
        }
        mCursorSelection.reset();
    } else {
        if (mMaxTextLength <= length())
            return;
        if (!typeableInsert(mCursorIndex, c)) {
            return;
        }
        ++mCursorIndex;
    }
    typeableInvalidateFont();
    updateCursorBlinking();
    onCursorIndexChanged();

    if (!AInput::isKeyDown(AInput::LSHIFT) && !AInput::isKeyDown(AInput::RSHIFT))
    {
        mCursorSelection.reset();
    }

    cursorSelectableRedraw();
}

AMenuModel AAbstractTypeable::composeContextMenuImpl() {
    return { { .name = "aui.cut"_i18n, .shortcut = AInput::LCONTROL + AInput::X, .onAction = [&]{cutToClipboard();}, .enabled = hasSelection(), },
             { .name = "aui.copy"_i18n, .shortcut = AInput::LCONTROL + AInput::C, .onAction = [&]{copyToClipboard();}, .enabled = hasSelection() },
             { .name = "aui.paste"_i18n, .shortcut = AInput::LCONTROL + AInput::V, .onAction = [&]{pasteFromClipboard();}, .enabled = !AClipboard::isEmpty() },
             { .type = AMenu::SEPARATOR, },
             { .name = "aui.select_all"_i18n, .shortcut = AInput::LCONTROL + AInput::A, .onAction = [&]{selectAll();}, .enabled = !getText().empty() } };
}

void AAbstractTypeable::setText(const AString& t)
{
    updateSelectionOnTextSet(t);
    updateCursorBlinking();
    onCursorIndexChanged();

    typeableInvalidateFont();
    emitTextChanged(t);
}

void AAbstractTypeable::updateSelectionOnTextSet(const AString& t) {
    mCursorIndex = t.length();
    mCursorSelection = 0;
}

void AAbstractTypeable::drawCursorImpl(IRenderer& renderer, glm::ivec2 position, unsigned int lineHeight) {
    if (!isCursorBlinkVisible()) {
        return;
    }
    renderer.setBlending(Blending::INVERSE_DST);
    AUI_DEFER { renderer.setBlending(Blending::NORMAL); };
    renderer.rectangle(ASolidBrush{}, position, {1, lineHeight});
}

void AAbstractTypeable::moveCursorLeft() {
    fastenSelection();
    if (mCursorIndex) {
        if (AInput::isKeyDown(AInput::LCONTROL) || AInput::isKeyDown(AInput::RCONTROL)) {
            if (mCursorIndex <= 1) {
                mCursorIndex = 0;
            } else {
                mCursorIndex = typeableReverseFind(' ', mCursorIndex - 2) + 1;
            }
        } else {
            mCursorIndex -= 1;
        }
    }
    onCursorIndexChanged();
    updateCursorBlinking();
    cursorSelectableRedraw();
}

void AAbstractTypeable::moveCursorRight() {
    fastenSelection();
    if (mCursorIndex < length()) {
        if (AInput::isKeyDown(AInput::LCONTROL) || AInput::isKeyDown(AInput::RCONTROL)) {
            auto index = typeableFind(' ', mCursorIndex);
            if (index == AString::NPOS) {
                mCursorIndex = length();
            } else {
                mCursorIndex = index + 1;
            }
        } else {
            mCursorIndex += 1;
        }
    }
    onCursorIndexChanged();
    updateCursorBlinking();
    cursorSelectableRedraw();
}
