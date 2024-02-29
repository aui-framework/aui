// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#pragma once


#include "ARender.h"
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
        void setPosition(const glm::ivec2& position) override;
        Float getFloat() const override;

        ~TextEntry() override = default;

        const glm::vec2& getPosition() const {
            return mPosition;
        }
    };


public:
    ARender::PrerenderedString updateText(const AString& text, const glm::ivec2& size);

    void setFontStyle(const AFontStyle& fontStyle) {
        mFontStyle = fontStyle;
    }
};


