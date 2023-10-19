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
// Created by Alex2772 on 11/20/2021.
//

#include "AMultilineTextRender.h"

glm::ivec2 AMultilineTextRender::TextEntry::getSize() {
    return {mTextRender.mFontStyle.getWidth(getText()), mTextRender.mFontStyle.size };
}

void AMultilineTextRender::TextEntry::setPosition(const glm::ivec2& position) {
    mPosition = position;
}

Float AMultilineTextRender::TextEntry::getFloat() const {
    return Float::NONE;
}


ARender::PrerenderedString AMultilineTextRender::updateText(const AString& text, const glm::ivec2& size) {
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


    auto multiStringCanvas = ARender::newMultiStringCanvas(mFontStyle);

    for (auto& entry : textEntries) {
        multiStringCanvas->addString(entry.getPosition(), entry.getText());
    }

    return multiStringCanvas->finalize();
}
