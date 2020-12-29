//
// Created by Алексей on 25.07.2018.
//

#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <AUI/Common/AVector.h>
#include <AUI/Url/AUrl.h>

class API_AUI_CORE AImage {
public:
	enum Format : unsigned {
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
	unsigned mFormat = UNKNOWN;
	
public:
	AImage();
	AImage(Format f);
	AImage(AVector<uint8_t> mData, uint16_t mWidth, uint16_t mHeight, int mFormat);

	void allocate() {
	    mData.resize(mWidth * mHeight * getBytesPerPixel());
	}

	AVector<uint8_t>& getData();

    [[nodiscard]] inline uint16_t getWidth() const {
	    return mWidth;
	}
    [[nodiscard]] inline uint16_t getHeight() const {
	    return mHeight;
	}

    inline unsigned getFormat() const {
        return mFormat;
    }

    /**
     * \return Количество байт на пиксель.
     */
    inline uint8_t getBytesPerPixel() const {
        auto b = static_cast<uint8_t>(mFormat & 15u);
        if (mFormat & FLOAT) {
            b *= 4;
        }
        return b;
    }


    inline glm::ivec2 getSize() const {
	    return {getWidth(), getHeight()};
	}

    glm::ivec4 getPixelAt(uint16_t x, uint16_t y);
    void setPixelAt(uint16_t x, uint16_t y, const glm::ivec4& val);

	static _<AImage> addAlpha(const _<AImage>& AImage);
	static _<AImage> resize(_<AImage> src, uint16_t width, uint16_t height);
	static _<AImage> resizeLinearDownscale(_<AImage> src, uint16_t width, uint16_t height);
	static void copy(_<AImage> src, _<AImage> dst, uint32_t x, uint32_t y);

    uint8_t& at(uint16_t x, uint16_t y) {
        return mData[(y * getWidth() + x) * getBytesPerPixel()];
    }
    const uint8_t& at(uint16_t x, uint16_t y) const {
        return mData[(y * getWidth() + x) * getBytesPerPixel()];
    }

    static _<AImage> fromUrl(const AUrl& url);
};

