#pragma once

#include <AUI/Url/AUrl.h>
#include "IImageLoader.h"
#include "AUI/Common/ADeque.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/Util/Singleton.h"

class API_AUI_CORE AImageLoaderRegistry: public Singleton<AImageLoaderRegistry>
{
private:
	ADeque<_<IImageLoader>> mImageLoaders;
	
public:
	AImageLoaderRegistry()
	{
	}

	void registerImageLoader(_<IImageLoader> imageLoader);

	_<IDrawable> loadVectorImage(_<AByteBuffer> buffer);
	_<AImage> loadRasterImage(_<AByteBuffer> buffer);
	inline _<IDrawable> loadVectorImage(const AUrl& url) {
	    return loadVectorImage(AByteBuffer::fromStream(url.open()));
	}
	inline _<AImage> loadRasterImage(const AUrl& url) {
        return loadRasterImage(AByteBuffer::fromStream(url.open()));
	}
};
