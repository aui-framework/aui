#include "Drawables.h"

#include "AImageLoaderRegistry.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Url/AUrl.h"

_<IDrawable> Drawables::load(const AString& key)
{
    try {
        auto buffer = AByteBuffer::fromStream(AUrl(key).open(), 0x100000);
        auto d = AImageLoaderRegistry::instance()
                .loadDrawable(buffer);
        if (d)
            return d;
        if (mImageToDrawable)
            if (auto raster = AImageLoaderRegistry::instance().loadImage(buffer))
                return mImageToDrawable(raster);
    } catch (const AException& e) {
        ALogger::err("Could not load image: " + key + ": " + e.getMessage());
    }
    return nullptr;
}

Drawables::~Drawables()
= default;
