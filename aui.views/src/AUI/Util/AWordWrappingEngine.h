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
#include <AUI/Enum/AFloat.h>

class AWordWrappingEngineBase {
public:
    class Entry {
    public:
        virtual ~Entry() = default;

        virtual glm::ivec2 getSize() = 0;
        virtual void setPosition(glm::ivec2 position) {}

        [[nodiscard]]
        virtual AFloat getFloat() const {
            return AFloat::NONE;
        }

        [[nodiscard]]
        virtual bool forcesNextLine() const {
            return false;
        }

        [[nodiscard]]
        bool isFloating() const {
            return getFloat() != AFloat::NONE;
        }

        [[nodiscard]]
        virtual bool escapesEdges() {
            return false;
        }
    };

protected:
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


    [[nodiscard]]
    AOptional<int> height() const {
        return mHeight;
    }
};

template<typename Container = AVector<_<AWordWrappingEngineBase::Entry>>>
class AWordWrappingEngine: public AWordWrappingEngineBase {
public:
    using Entries = Container;

    // include AWordWrappingEngineImpl.h for implementation
    void performLayout(const glm::ivec2& offset, const glm::ivec2& size);

    void setEntries(Container entries) {
        mEntries = std::move(entries);
    }

    [[nodiscard]]
    Container& entries() {
        return mEntries;
    }


    [[nodiscard]]
    const Container& entries() const {
        return mEntries;
    }

private:
    Container mEntries;
};


