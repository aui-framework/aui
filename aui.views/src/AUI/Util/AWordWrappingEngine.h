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

#pragma once


#include <AUI/Common/AObject.h>
#include <AUI/Common/AVector.h>
#include <glm/glm.hpp>
#include <AUI/Enum/TextAlign.h>
#include <AUI/Enum/Float.h>

class API_AUI_VIEWS AWordWrappingEngine {
public:
    class Entry {
    public:
        virtual ~Entry() = default;

        virtual glm::ivec2 getSize() = 0;
        virtual void setPosition(const glm::ivec2& position) = 0;

        [[nodiscard]]
        virtual Float getFloat() const = 0;

        [[nodiscard]]
        bool isFloating() const {
            return getFloat() != Float::NONE;
        }

        [[nodiscard]]
        virtual bool escapesEdges() {
            return false;
        }
    };

private:
    AVector<_<Entry>> mEntries;
    float mLineHeight = 1.f;
    TextAlign mTextAlign = TextAlign::LEFT;

public:
    void setLineHeight(float lineHeight) {
        mLineHeight = lineHeight;
    }

    void setTextAlign(TextAlign textAlign) {
        mTextAlign = textAlign;
    }

    void setEntries(AVector<_<Entry>> entries) {
        mEntries = std::move(entries);
    }

    void performLayout(const glm::ivec2& offset, const glm::ivec2& size);
};


