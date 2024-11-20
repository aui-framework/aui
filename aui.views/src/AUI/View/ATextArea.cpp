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


#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/AMetric.h>
#include "ATextArea.h"
#include <optional>
#include <glm/ext/matrix_transform.hpp>


ATextArea::ATextArea() {
    addAssName(".input-field");
}

ATextArea::ATextArea(const AString& text):
    ATextArea()
{
    setText(text);
}

ATextArea::~ATextArea() {

}

AString ATextArea::toString() const {
    return AString();
}

void ATextArea::invalidateFont() {

}

const AString& ATextArea::text() const {
    return *mText;
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

size_t ATextArea::typeableReverseFind(char16_t c, size_t startPos) {
    return 0;
}

size_t ATextArea::length() const {
    return 0;
}