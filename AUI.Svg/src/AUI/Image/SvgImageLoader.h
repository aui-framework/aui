#pragma once
#include "AUI/Image/IImageLoader.h"

class SvgImageLoader: public IImageLoader
{
public:
	SvgImageLoader();
	virtual ~SvgImageLoader() = default;

	bool matches(AByteBuffer& buffer) override;
	_<IDrawable> getDrawable(AByteBuffer& buffer) override;

    _<AImage> getRasterImage(AByteBuffer& buffer) override;
};
