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
// Created by Alex2772 on 11/20/2021.
//

#include "AMultilineTextRender.h"

glm::ivec2 AMultilineTextRender::TextEntry::getSize() {
    return {mTextRender.mFontStyle.getWidth(getText()), mTextRender.mFontStyle.size };
}

void AMultilineTextRender::TextEntry::setPosition(const glm::ivec2& position) {
    mPosition = position;
}

AFloat AMultilineTextRender::TextEntry::getFloat() const {
    return AFloat::NONE;
}


_<IRenderer::IPrerenderedString> AMultilineTextRender::updateText(const AString& text, const glm::ivec2& size, IRenderer& render) {
    AVector<TextEntry> textEntries;

    auto lastWordBegin = text.begin();
    for (auto it = text.begin(); it != text.end(); ++it) {
        if (*it == ' ') {
            textEntries.emplace_back(lastWordBegin, it, *this);
            ++it;
            lastWordBegin = it;
        }
    }
    if (lastWordBegin != text.end()) {
        textEntries.emplace_back(lastWordBegin, text.end(), *this);
    }

    // compose entries
    AVector<_<AWordWrappingEngine::Entry>> entries;
    entries.reserve(textEntries.size());

    for (auto& textEntry : textEntries) {
        entries << aui::ptr::fake(&textEntry);
    }
    mEngine.setEntries(std::move(entries));
    mEngine.setTextAlign(mFontStyle.align);
    mEngine.performLayout({0, 0}, size);


    auto multiStringCanvas = render.newMultiStringCanvas(mFontStyle);

    for (auto& entry : textEntries) {
        multiStringCanvas->addString(entry.getPosition(), entry.getText());
    }

    return multiStringCanvas->finalize();
}
