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

#pragma once


#include "IRenderer.h"
#include <AUI/Util/AWordWrappingEngine.h>
#include "AUI/Font/AFontStyle.h"

/**
 * Helper class for rendering and processing multiline texts.
 */
class AMultilineTextRender {
private:
friend class TextEntry;
    AWordWrappingEngine mEngine;
    AFontStyle mFontStyle;

    class TextEntry: public AWordWrappingEngine::Entry {
    private:
        AString::const_iterator mBegin;
        AString::const_iterator mEnd;
        AMultilineTextRender& mTextRender;
        glm::vec2 mPosition;

    public:
        TextEntry(const AString::const_iterator& begin, const AString::const_iterator& end, AMultilineTextRender& textRender)
                : mBegin(begin), mEnd(end), mTextRender(textRender) {}

        AString getText() const {
            return {mBegin, mEnd};
        }
        glm::ivec2 getSize() override;
        void setPosition(glm::ivec2 position) override;
        AFloat getFloat() const override;

        ~TextEntry() override = default;

        const glm::vec2& getPosition() const {
            return mPosition;
        }
    };


public:
    _<IRenderer::IPrerenderedString> updateText(const AString& text, const glm::ivec2& size, IRenderer& render);

    void setFontStyle(const AFontStyle& fontStyle) {
        mFontStyle = fontStyle;
    }
};


