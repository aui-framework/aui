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

#include <AUI/Platform/ACursor.h>
#include "AUI/Traits/callables.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Platform/AWindow.h"



ACursor::~ACursor() {

}

ACursor::ACursor(aui::non_null<_<IDrawable>> drawable, int size): mValue(std::move(drawable)), mSize(size) {
}

ACursor::ACursor(AUrl imageUrl, int size): mValue([&] {
    if (auto image = IDrawable::fromUrl(imageUrl)) {
        return image;
    }
    throw AException("bad cursor url: {} (please check [Drawable] log)"_format(imageUrl.full()));
}()), mSize(size) {

}

