#include "SimpleTexturePacker.h"


Util::SimpleTexturePacker::SimpleTexturePacker() {

}

Util::SimpleTexturePacker::~SimpleTexturePacker() {

}

void Util::SimpleTexturePacker::onResize(_<AImage> data, Util::dim side) {
	if (mImage) {
		mImage = AImage::resize(mImage, static_cast<uint16_t>(side), static_cast<uint16_t>(side));
	}
	else {
		AVector<uint8_t> c;
		c.resize(static_cast<size_t>(side * side * data->getBPP()));
		mImage = _new<AImage>(c, side, side, data->getFormat());
	}
}

void
Util::SimpleTexturePacker::onInsert(_<AImage> data, const Util::dim& x, const Util::dim& y) {
	AImage::copy(data, mImage, x, y);
}

_<glm::vec4> Util::SimpleTexturePacker::insert(_<AImage> data) {
	return TexturePacker::insert(data, (dim)data->getWidth(), (dim)data->getHeight());
}

_<AImage> Util::SimpleTexturePacker::getImage() {
	return mImage;
}
