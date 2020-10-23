#pragma once
#include "IDrawable.h"
#include "AUI/Util/Cache.h"
#include "AImage.h"

class API_AUI_CORE Drawables: public Cache<IDrawable, Drawables>
{
private:
    _<IDrawable>(*mImageToDrawable)(const _<AImage>& image) = nullptr;

public:
	_<IDrawable> load(const AString& key) override;
	~Drawables() override;

    static void setImageToDrawable(_<IDrawable>(*imageToDrawable)(const _<AImage>&)) {
        instance().mImageToDrawable = imageToDrawable;
    }
};
