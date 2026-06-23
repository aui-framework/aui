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
 * @brief Image loader used for IDrawable::fromUrl and AImage::fromUrl.
 * @ingroup image
 */
class API_AUI_IMAGE AImageLoaderRegistry {
    friend class AImage::Cache;
    friend class IDrawable;
    friend class AImage;

private:
    ADeque<AArc<IImageLoader>> mRasterLoaders;
    ADeque<AArc<IImageLoader>> mVectorLoaders;
    ADeque<AArc<IImageLoader>> mAnimatedLoaders;
    ADeque<AString> mSupportedFormats;

    AArc<IImageFactory> loadVector(AByteBufferView buffer);
    AArc<IAnimatedImageFactory> loadAnimated(AByteBufferView buffer);
    AArc<AImage> loadRaster(AByteBufferView buffer);
    inline AArc<IImageFactory> loadVector(const AUrl& url) {
        auto s = AByteBuffer::fromStream(url.open());
        return loadVector(s);
    }
    AArc<AImage> loadImage(const AUrl& url);

    void registerLoader(ADeque<AArc<IImageLoader>>& d, AArc<IImageLoader> loader, AString name);

public:
    AImageLoaderRegistry() = default;

    void registerRasterLoader(AArc<IImageLoader> imageLoader, AString name = "");
    void registerVectorLoader(AArc<IImageLoader> imageLoader, AString name = "");
    void registerAnimatedLoader(AArc<IImageLoader> imageLoader, AString name = "");
    [[nodiscard]]
    const ADeque<AString>& supportedFormats() const noexcept;

    static AImageLoaderRegistry& inst();
};
