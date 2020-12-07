#pragma once

#include <AUI/Url/AUrl.h>
#include <AUI/Logging/ALogger.h>
#include "IImageLoader.h"
#include "AUI/Common/ADeque.h"
#include "AUI/Common/SharedPtr.h"

class API_AUI_CORE AImageLoaderRegistry
{
private:
	ADeque<_<IImageLoader>> mImageLoaders;
	
public:
	AImageLoaderRegistry()
	{
	}

	void registerImageLoader(_<IImageLoader> imageLoader);

	_<IDrawable> loadDrawable(AByteBuffer& buffer);
	_<AImage> loadImage(AByteBuffer& buffer);
	inline _<IDrawable> loadDrawable(const AUrl& url) {
	    auto s = AByteBuffer::fromStream(url.open());
	    return loadDrawable(s);
	}
	_<AImage> loadImage(const AUrl& url);

	static AImageLoaderRegistry& inst();
};
