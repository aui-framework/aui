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

