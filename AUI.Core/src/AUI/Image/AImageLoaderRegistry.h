#pragma once
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

	_<IDrawable> loadImage(_<ByteBuffer> buffer);
};
