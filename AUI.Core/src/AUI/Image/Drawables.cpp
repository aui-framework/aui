#include "Drawables.h"

#include "AImageLoaderRegistry.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Url/AUrl.h"

_<IDrawable> Drawables::load(const AString& key)
{
	return AImageLoaderRegistry::instance()
            .loadVectorImage(AByteBuffer::fromStream(AUrl(key).open()));
}

Drawables::~Drawables()
= default;
