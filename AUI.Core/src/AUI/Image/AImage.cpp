//
// Created by Алексей on 25.07.2018.
//

#include <utility>
#include <cstring>
#include "AImage.h"
#include "AImageLoaderRegistry.h"

AImage::AImage() {

}

AImage::AImage(AImage::Format f) :
	mFormat(f)
{

}

AImage::AImage(AVector<uint8_t> mData, uint16_t mWidth, uint16_t mHeight, int mFormat) : mData(std::move(
	                                                                                     mData)), mWidth(mWidth), mHeight(mHeight), mFormat(mFormat) {

}

AVector<uint8_t>& AImage::getData() {
	return mData;
}

_<AImage> AImage::addAlpha(const _<AImage>& src)
{
	auto dst = _new<AImage>(AVector<uint8_t>(), src->getWidth(), src->getHeight(), RGBA | BYTE);
	dst->getData().resize(src->getWidth() * src->getHeight() * dst->getBytesPerPixel());
	memset(dst->getData().data(), 255, dst->getData().size());

	// https://stackoverflow.com/questions/9900854/opengl-creating-texture-atlas-at-run-time
	for (int sourceY = 0; sourceY < src->mHeight; ++sourceY) {
		for (int sourceX = 0; sourceX < src->mWidth; ++sourceX) {
			int from = (sourceY * src->mWidth * src->getBytesPerPixel()) + (sourceX * src->getBytesPerPixel()); // 4 bytes per pixel (assuming RGBA)
			int to = ((sourceY)* dst->mWidth * dst->getBytesPerPixel()) + ((sourceX)* dst->getBytesPerPixel()); // same format as source

			for (int channel = 0; channel < src->getBytesPerPixel(); ++channel) {
				dst->mData[to + channel] = src->mData[from + channel];
			}
		}
	}
	return dst;
}

_<AImage> AImage::resize(_<AImage> src, uint16_t width, uint16_t height) {
	auto n = _new<AImage>(AVector<uint8_t>(), width, height, src->getFormat());
	n->getData().resize(width * height * n->getBytesPerPixel());
	copy(src, n, 0, 0);
	return n;
}

glm::ivec4 AImage::getPixelAt(uint16_t x, uint16_t y)
{
	uint8_t* dataPtr = &mData[mHeight * glm::clamp(y, uint16_t(0), mHeight) + glm::clamp(x, uint16_t(0), mWidth)];
	switch (getBytesPerPixel())
	{
	case 1:
		return { dataPtr[0], 0, 0, 0 };
	case 2:
		return { dataPtr[0], dataPtr[1], 0, 0 };
	case 3:
		return { dataPtr[0], dataPtr[1], dataPtr[2], 0 };
	case 4:
		return { dataPtr[0], dataPtr[1], dataPtr[2], dataPtr[3] };
	}
	return {};
}
void AImage::setPixelAt(uint16_t x, uint16_t y, const glm::ivec4& val)
{
	uint8_t* dataPtr = &mData[mHeight * glm::clamp(y, uint16_t(0), mHeight) + glm::clamp(x, uint16_t(0), mWidth)];
	switch (getBytesPerPixel())
	{
	case 4:
		dataPtr[3] = val.w;
	case 3:
		dataPtr[2] = val.z;
	case 2:
		dataPtr[1] = val.y;
	case 1:
		dataPtr[0] = val.x;
	}
}

_<AImage> AImage::resizeLinearDownscale(_<AImage> src, uint16_t width, uint16_t height)
{
	auto n = _new<AImage>(AVector<uint8_t>(), width, height, src->getFormat());
	n->getData().resize(width * height * n->getBytesPerPixel());

	uint16_t deltaX = src->getWidth() / width;
	uint16_t deltaY = src->getHeight() / height;

	for (uint16_t y = 0; y < height; ++y)
	{
		for (uint16_t x = 0; x < width; ++x)
		{
			glm::ivec4 block;
			for (uint16_t dy = 0; dy < deltaY; ++dy)
			{
				for (uint16_t dx = 0; dx < deltaX; ++dx)
				{
					block += src->getPixelAt(x * deltaX + dx, y * deltaY + dy);
				}
			}
			block /= deltaY * deltaX;
			n->setPixelAt(x, y, block);
		}
	}

	return n;
}

void AImage::copy(_<AImage> src, _<AImage> dst, uint32_t x, uint32_t y) {
	assert(src->getFormat() == dst->getFormat());
	
	// https://stackoverflow.com/questions/9900854/opengl-creating-texture-atlas-at-run-time
	for (uint32_t sourceY = 0; sourceY < glm::min(src->mHeight, dst->mHeight); ++sourceY) {
		for (uint32_t sourceX = 0; sourceX < glm::min(src->mWidth, dst->mWidth); ++sourceX) {
			uint32_t from = (sourceY * uint32_t(src->mWidth) * uint32_t(src->getBytesPerPixel())) + (sourceX * uint32_t(src->getBytesPerPixel())); // 4 bytes per pixel (assuming RGBA)
			uint32_t to = ((y + sourceY) *uint32_t(dst->mWidth) * uint32_t(dst->getBytesPerPixel())) + ((x + sourceX) * uint32_t(dst->getBytesPerPixel())); // same format as source

			for (uint32_t channel = 0; channel < dst->getBytesPerPixel(); ++channel) {
				dst->mData[to + channel] = src->mData[from + channel];
			}
		}
	}
}

_<AImage> AImage::fromUrl(const AUrl& url) {
    return AImageLoaderRegistry::inst().loadImage(url);
}

