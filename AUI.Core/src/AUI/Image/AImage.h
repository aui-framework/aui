//
// Created by Алексей on 25.07.2018.
//

#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "AUI/Common/AVector.h"

class API_AUI_CORE AImage {
public:
	enum Format {
		UNKNOWN = 0,
		R = 1,
		RG = 2,
		RGB = 3,
		RGBA = 4,
		FLOAT = 8,
		BYTE = 16
	};
private:
	AVector<uint8_t> mData;
	uint16_t mWidth;
	uint16_t mHeight;
	int mFormat = UNKNOWN;
	
public:
	AImage();
	AImage(Format f);
	AImage(AVector<uint8_t> mData, uint16_t mWidth, uint16_t mHeight, int mFormat);

	AVector<uint8_t>& getData();
	uint16_t getWidth() const;
	uint16_t getHeight() const;
	uint8_t getBPP() const;

	int getFormat() const;
	static _<AImage> addAlpha(const _<AImage>& AImage);
	static _<AImage> resize(_<AImage> src, uint16_t width, uint16_t height);
	glm::ivec4 getPixelAt(uint16_t x, uint16_t y);
	void setPixelAt(uint16_t x, uint16_t y, const glm::ivec4& val);
	static _<AImage> resizeLinearDownscale(_<AImage> src, uint16_t width, uint16_t height);
	static void copy(_<AImage> src, _<AImage> dst, uint16_t x, uint16_t y);
};

