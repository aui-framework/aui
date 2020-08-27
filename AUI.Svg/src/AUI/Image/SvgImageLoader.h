#pragma once
#include "AUI/Image/IImageLoader.h"

class SvgImageLoader: public IImageLoader
{
public:
	SvgImageLoader();
	virtual ~SvgImageLoader() = default;

	bool matches(_<ByteBuffer> buffer) override;
	_<IDrawable> getDrawable(_<ByteBuffer> buffer) override;

    _<AImage> getRasterImage(_<ByteBuffer> buffer) override;
};
