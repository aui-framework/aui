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
// Created by alex2 on 30.11.2020.
//

#include <AUI/Platform/AInput.h>
#include "AUI/Render/IRenderer.h"
#include <AUI/Render/RenderHints.h>
#include "ACursorSelectable.h"

ACursorSelectable::~ACursorSelectable() = default;

ACursorSelectable::Selection ACursorSelectable::selection() const {
    return { glm::min(mCursorIndex, mCursorSelection.valueOr(mCursorIndex)), glm::max(mCursorIndex, mCursorSelection.valueOr(mCursorIndex)) };
}

bool ACursorSelectable::hasSelection() const {
    return mCursorIndex != mCursorSelection && mCursorSelection.hasValue();
}


void ACursorSelectable::handleMousePressed(const APointerPressedEvent& event) {
    if (event.pointerIndex != APointerIndex::button(AInput::RBUTTON)) {
        mCursorSelection = mCursorIndex = cursorIndexByPos(event.position);
        onSelectionChanged();
    }
}

void ACursorSelectable::handleMouseMove(const glm::ivec2& pos) {
    if (!mIgnoreSelection && isLButtonPressed()) {
        mCursorIndex = cursorIndexByPos(pos);
        onSelectionChanged();
        cursorSelectableRedraw();
    }
}

void ACursorSelectable::handleMouseReleased(const APointerReleasedEvent& event) {
    if (event.pointerIndex != APointerIndex::button(AInput::RBUTTON))
    {
        mIgnoreSelection = false;
        if (mCursorSelection == mCursorIndex)
        {
            mCursorSelection.reset();
        }
        onSelectionChanged();
    }
}

void ACursorSelectable::selectAll() {
    size_t l = length();
    if (l > 0) {
        mCursorSelection = 0;
        mCursorIndex = l;
        onSelectionChanged();
    }
}

void ACursorSelectable::clearSelection() {
    mCursorSelection.reset();
}


void ACursorSelectable::handleMouseDoubleClicked(const APointerPressedEvent& event) {
    mIgnoreSelection = true;
    auto text = getDisplayText();

    // select word
    auto clickIndex = cursorIndexByPos(event.position);


    // determine the begin and end indices of the word
    auto predicate = [](auto c) { return c == ' ' || c == '\n'; };
    auto end = std::find_if(text.bytes().begin() + clickIndex, text.bytes().end(), predicate);
    auto begin = std::find_if(std::make_reverse_iterator(text.bytes().begin() + clickIndex), text.bytes().rend(), predicate).base();

    mCursorSelection = std::distance(text.bytes().begin(), begin);
    mCursorIndex = std::distance(text.bytes().begin(), end);
    onSelectionChanged();
}

