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


#include "ATextArea.h"
#include "AUI/Traits/algorithms.h"


ATextArea::ATextArea() {
    addAssName(".input-field");
}

ATextArea::ATextArea(const AString& text):
    ATextArea()
{
    setText(text);
}

void ATextArea::setText(const AString& t) {
    AAbstractTextView::setString(t);
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

bool ATextArea::typeableInsert(size_t at, char16_t toInsert) {
    return false;
}

size_t ATextArea::typeableFind(char16_t c, size_t startPos) {
    return 0;
}

void ATextArea::onCharEntered(char16_t c) {
    AAbstractTypeableView<AAbstractTextView>::onCharEntered(c);
    enterChar(c);
    if (textChanging) emit textChanging(text());
}

size_t ATextArea::typeableReverseFind(char16_t c, size_t startPos) {
    return 0;
}

size_t ATextArea::length() const {
    return 0;
}

unsigned int ATextArea::cursorIndexByPos(glm::ivec2 pos) {
    return 0;
}

glm::ivec2 ATextArea::getPosByIndex(int i) {
    return glm::ivec2();
}

void ATextArea::onCursorIndexChanged() {
    mCursorPosition = getPosByIndex(mCursorIndex);
}

void ATextArea::render(ARenderContext context) {
    AAbstractTextView::render(context);
    drawCursor(context.render, mCursorPosition + mPadding.leftTop());
}

bool ATextArea::capturesFocus() {
    return true;
}
