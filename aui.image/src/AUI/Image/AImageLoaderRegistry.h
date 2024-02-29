// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <AUI/Url/AUrl.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/Cache.h>
#include "IImageLoader.h"
#include "AUI/Common/ADeque.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/Image/IAnimatedImageFactory.h"

/**
 * Image loader used for IDrawable::fromUrl and AImage::fromUrl
 */
class API_AUI_IMAGE AImageLoaderRegistry
{
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
