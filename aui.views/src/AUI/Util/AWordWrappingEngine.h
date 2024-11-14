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


#include <AUI/Common/AObject.h>
#include <AUI/Common/AVector.h>
#include <glm/glm.hpp>
#include <AUI/Enum/ATextAlign.h>
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
    ATextAlign mTextAlign = ATextAlign::LEFT;
    AOptional<int> mHeight;

public:
    void setLineHeight(float lineHeight) {
        mLineHeight = lineHeight;
    }

    void setTextAlign(ATextAlign textAlign) {
        mTextAlign = textAlign;
    }

    void setEntries(AVector<_<Entry>> entries) {
        mEntries = std::move(entries);
    }

    [[nodiscard]]
    const AVector<_<Entry>>& entries() const {
        return mEntries;
    }

    [[nodiscard]]
    AOptional<int> height() const {
        return mHeight;
    }

    void performLayout(const glm::ivec2& offset, const glm::ivec2& size);
};


