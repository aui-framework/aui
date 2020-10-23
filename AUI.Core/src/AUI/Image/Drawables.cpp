#include "Drawables.h"

#include "AImageLoaderRegistry.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Url/AUrl.h"

_<IDrawable> Drawables::load(const AString& key)
{
    auto buffer = AByteBuffer::fromStream(AUrl(key).open());
	auto d =  AImageLoaderRegistry::instance()
            .loadDrawable(buffer);
	if (d)
	    return d;
    if (mImageToDrawable)
	    return mImageToDrawable(AImageLoaderRegistry::instance().loadImage(buffer));
    return nullptr;
}

Drawables::~Drawables()
= default;
