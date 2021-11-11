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
// Created by alex2 on 7/1/2021.
//


#pragma once

#include <AUI/Common/ASignal.h>
#include "AUI/Common/AVariant.h"
#include "AModelRange.h"

class ATreeIndex {
private:
    void* mUserData;

public:
    ATreeIndex() = default;

    explicit ATreeIndex(void* userData):
            mUserData(userData)
    {
    }

    template<typename T>
    [[nodiscard]]
    T* getUserData() const {
        return reinterpret_cast<T*>(mUserData);
    }
};

template<typename T>
class ITreeModel
{
protected:
    virtual void* rootUserData() = 0;

public:
    virtual ~ITreeModel() = default;

    virtual size_t childrenCount(const ATreeIndex& parent) = 0;
    virtual T itemAt(const ATreeIndex& index) = 0;
    virtual ATreeIndex indexOfChild(size_t row, size_t column, const ATreeIndex& parent) = 0;

    ATreeIndex root() {
        return ATreeIndex{rootUserData()};
    }

    using stored_t = T;

signals:
};
