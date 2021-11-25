//
// Created by Alex2772 on 11/17/2021.
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


Render::PrerenderedString AMultilineTextRender::updateText(const AString& text, const glm::ivec2& size) {
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


    auto multiStringCanvas = Render::newMultiStringCanvas(mFontStyle);

    for (auto& entry : textEntries) {
        multiStringCanvas->addString(entry.getPosition(), entry.getText());
    }

    return multiStringCanvas->build();
}
