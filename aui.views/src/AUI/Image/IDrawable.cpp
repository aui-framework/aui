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

        ALogger::err("Drawable") << "Image of unknown format: " << key.full() << ", AUI-supported formats: " << AImageLoaderRegistry::inst().supportedFormats();
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
    AUI_ASSERT_NO_CONDITION("unimplemented");
    throw AException("unimplemented");
}
