#include "Drawables.h"

#include "AImageLoaderRegistry.h"
#include "AUI/Common/ByteBuffer.h"
#include "AUI/Url/AUrl.h"

_<IDrawable> Drawables::load(const AString& key)
{
	return AImageLoaderRegistry::instance()
			.loadImage(ByteBuffer::fromStream(AUrl(key).open()));
}

Drawables::~Drawables()
= default;
