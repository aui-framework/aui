#pragma once

#include "TexturePacker.h"
#include "AUI/Image/AImage.h"

namespace Util {
	class SimpleTexturePacker : public ::Util::TexturePacker<AImage> {
	private:
		_<AImage> mImage;
	public:
		SimpleTexturePacker();
		~SimpleTexturePacker();
		_<glm::vec4> insert(_<AImage> data) override;
		SimpleTexturePacker(const SimpleTexturePacker&) = delete;

		_<AImage> getImage();
	protected:
		void onResize(_<AImage> data, dim side) override;
		void onInsert(_<AImage> data, const dim& x, const dim& y) override;
	};
}



