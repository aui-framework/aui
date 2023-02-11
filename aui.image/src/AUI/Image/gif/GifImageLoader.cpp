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

//
// Created by dervisdev on 1/11/2023.
//

#include "GifImageLoader.h"
#include "GifImageFactory.h"

bool GifImageLoader::matches(AByteBufferView buffer) {
    return memcmp(buffer.data(), "GIF87a", 6) == 0 ||
            memcmp(buffer.data(), "GIF89a", 6) == 0;
}

_<IImageFactory> GifImageLoader::getImageFactory(AByteBufferView buffer) {
    return _new<GifImageFactory>(buffer);
}

_<AImage> GifImageLoader::getRasterImage(AByteBufferView buffer) {
    return nullptr;
}