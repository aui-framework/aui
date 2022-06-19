/*
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

#pragma once

#include <glm/glm.hpp>
#include "AUI/Common/AVector.h"
#include "AUI/Common/SharedPtr.h"

namespace Util {
    typedef int dim;
    class Rect {
    public:
        dim x, y, width, height;
        Rect(dim x, dim y, dim width, dim height);
        Rect() = default;
        bool hasPoint(dim x, dim y) const;
        bool collidesWith(const Rect& rect);
    };
    class TexturePacker_Lol {
    protected:
        dim side;
        AVector<Rect> mRects;
        bool noCollision(const Rect& r);
        bool check(Rect& res, Rect r);
        bool allocateRect(Rect& r, dim width, dim height);
    };

    template<class T>
    class TexturePacker : public TexturePacker_Lol {
    protected:
        glm::vec4 insert(T& data, dim width, dim height)
        {
            if (side == 0)
                resize(data, 64);
            Rect r(0, 0, 0, 0);
            while (!allocateRect(r, width, height))
            {
                resize(data, side * 2);
            }
            this->onInsert(data, r.x, r.y);
            return { float(r.x), float(r.y), float(r.x + r.width), float(r.y + r.height) };
        }

        virtual void onResize(T& data, dim side) = 0;
        virtual void onInsert(T& data, const dim& x, const dim& y) = 0;

    public:
        TexturePacker()
        {
            side = 0;
        }
        virtual ~TexturePacker() {

        }

        virtual glm::vec4 insert(T& data) = 0;

        void resize(T& data, dim size)
        {
            side = size;
            this->onResize(data, size);
        }
    };
}

