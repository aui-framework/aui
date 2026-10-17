/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Url/AUrl.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/Cache.h>
#include "IImageLoader.h"
#include "AUI/Common/ADeque.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/Image/IAnimatedImageFactory.h"

/**
 * @brief Registry of [IImageLoader]s used by `IDrawable::fromUrl` and `AImage::fromUrl`.
 * @ingroup image
 * @details
 * The singleton `AImageLoaderRegistry::inst()` is populated by aui.image on startup with loaders for bmp, png, jpg,
 * webp, svg (vector) and gif/animated webp (animated). `supportedFormats()` lists their names.
 *
 * Loaders are tried in registration order; the first one whose `matches` returns true wins. Register your own with
 * `registerRasterLoader`, `registerVectorLoader` or `registerAnimatedLoader`. See [image-custom-loader].
 */
class API_AUI_IMAGE AImageLoaderRegistry {
    friend class AImage::Cache;
    friend class IDrawable;
    friend class AImage;

private:
    ADeque<_<IImageLoader>> mRasterLoaders;
    ADeque<_<IImageLoader>> mVectorLoaders;
    ADeque<_<IImageLoader>> mAnimatedLoaders;
    ADeque<AString> mSupportedFormats;

    _<IImageFactory> loadVector(AByteBufferView buffer);
    _<IAnimatedImageFactory> loadAnimated(AByteBufferView buffer);
    _<AImage> loadRaster(AByteBufferView buffer);
    inline _<IImageFactory> loadVector(const AUrl& url) {
        auto s = AByteBuffer::fromStream(url.open());
        return loadVector(s);
    }
    _<AImage> loadImage(const AUrl& url);

    void registerLoader(ADeque<_<IImageLoader>>& d, _<IImageLoader> loader, AString name);

public:
    AImageLoaderRegistry() = default;

    void registerRasterLoader(_<IImageLoader> imageLoader, AString name = "");
    void registerVectorLoader(_<IImageLoader> imageLoader, AString name = "");
    void registerAnimatedLoader(_<IImageLoader> imageLoader, AString name = "");
    [[nodiscard]]
    const ADeque<AString>& supportedFormats() const noexcept;

    static AImageLoaderRegistry& inst();
};
