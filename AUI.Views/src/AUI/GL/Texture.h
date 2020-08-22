#pragma once

#include "gl.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/Image/AImage.h"

namespace GL {
	class Texture {
	private:
		uint32_t mTexture = 0;
	public:
		Texture();
		~Texture();
		static void setupLinear();
		Texture(const Texture&) = delete;
		void bind(uint8_t index = 0) const;
		void tex2D(_<AImage> image);
		operator bool() const;
		uint32_t getHandle();
	};
}


