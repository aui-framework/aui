#pragma once

#include "AUI/Common/SharedPtr.h"
#include "AUI/Image/AImage.h"
#include "Texture.h"

namespace GL {
	class API_AUI_VIEWS Texture2D: public Texture<TEXTURE_2D> {
	public:
		void tex2D(const _<AImage>& image);
        virtual ~Texture2D() = default;
	};
}
