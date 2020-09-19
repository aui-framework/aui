#pragma once
#include "AUI/Image/IImageLoader.h"

class SvgImageLoader: public IImageLoader
{
public:
	SvgImageLoader();
	virtual ~SvgImageLoader() = default;

	bool matches(_<AByteBuffer> buffer) override;
	_<IDrawable> getDrawable(_<AByteBuffer> buffer) override;

    _<AImage> getRasterImage(_<AByteBuffer> buffer) override;
};
