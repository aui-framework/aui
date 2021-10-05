/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 5/15/2021.
//


#pragma once


#include <AUI/Platform/AInput.h>
#include <AUI/Common/AVector.h>

class AShortcut {
private:
    AVector<AInput::Key> mKeys;

public:
    AShortcut() = default;
    AShortcut(const AVector<AInput::Key>& keys) : mKeys(keys) {}
    AShortcut(AVector<AInput::Key>&& keys) : mKeys(keys) {}

    const AVector<AInput::Key>& getKeys() const {
        return mKeys;
    }

    operator AString() const {
        AString s;
        for (auto& k : mKeys) {
            if (!s.empty()) {
                s += "+";
            }
            s += AInput::getName(k);
        }
        return s;
    }
    bool empty() const {
        return mKeys.empty();
    }
};

inline AShortcut operator+(const AInput::Key& k1, const AInput::Key& k2) {
    return AShortcut({k1, k2});
}

inline AShortcut operator+(const AShortcut& k1, const AInput::Key& k2) {
    auto k = k1.getKeys();
    k << k2;
    return AShortcut(std::move(k));
}

