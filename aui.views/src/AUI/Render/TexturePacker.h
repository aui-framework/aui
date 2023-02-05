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

