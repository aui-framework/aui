//
// Created by Alex2772 on 11/17/2021.
//

#include "AMultilineTextRender.h"

glm::ivec2 AMultilineTextRender::TextEntry::getSize() {
    return {mTextRender.mFontStyle.getWidth(getText()), mTextRender.mFontStyle.size };
}

void AMultilineTextRender::TextEntry::setPosition(const glm::ivec2& position) {

}

Float AMultilineTextRender::TextEntry::getFloat() const {
    return Float::NONE;
}

Render::PrerenderedString AMultilineTextRender::updateText(const AString& text, const glm::ivec2& size) {
    AVector<_<AWordWrappingEngine::Entry>> entries;

    auto lastWordBegin = text.begin();
    for (auto it = text.begin(); it != text.end(); ++it) {
        if (*it == ' ') {
            entries << _new<TextEntry>(lastWordBegin, it, *this);
            ++it;
            lastWordBegin = it;
        }
    }
    if (lastWordBegin != text.end()) {
        entries << _new<TextEntry>(lastWordBegin, text.end(), *this);
    }

    mEngine.setEntries(entries);

    mEngine.performLayout({0, 0}, size);

    return nullptr;
}
