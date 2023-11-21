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

#include "IDrawable.h"
#include "AVectorDrawable.h"
#include "AAnimatedDrawable.h"
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/Util/AImageDrawable.h>


_<IDrawable> IDrawable::fromUrl(const AUrl& url) noexcept {
    return Cache::get(url);
}

_<IDrawable> IDrawable::Cache::load(const AUrl& key)
{
    try {
        auto buffer = AByteBuffer::fromStream(AUrl(key).open());

        if (auto vec = AImageLoaderRegistry::inst().loadVector(buffer)) {
            return _new<AVectorDrawable>(vec);
        }

        if (auto animated = AImageLoaderRegistry::inst().loadAnimated(buffer)) {
            return _new<AAnimatedDrawable>(animated);
        }

        if (auto raster = AImageLoaderRegistry::inst().loadRaster(buffer)) {
            return _new<AImageDrawable>(raster);
        }

        ALogger::err("Drawable") << "Could not recognize image: " << key.full();
    } catch (const AException& e) {
        ALogger::err("Drawable") << "Could not load image: " + key.full() + ": " << e;
    }
    return nullptr;
}

IDrawable::Cache& IDrawable::Cache::inst() {
    static IDrawable::Cache s;
    return s;
}

AImage IDrawable::rasterize(glm::ivec2 imageSize) {
    assert(("unimplemented", false));
    throw AException("unimplemented");
}
