#include "BuiltinFiles.h"

#include "LZ.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/ByteBufferInputStream.h"

void BuiltinFiles::loadBuffer(ByteBuffer& data)
{
	ByteBuffer unpacked;
	LZ::decompress(data, unpacked);

	while (unpacked.getAvailable())
	{
		std::string file;
		unpacked >> file;

		uint32_t s;
		unpacked >> s;

		auto b = _new<ByteBuffer>();
		b->reserve(s);
		b->setSize(s);

		unpacked.get(b->data(), s);
		instance().mBuffers[AString(file)] = b;
	}
}

_<IInputStream> BuiltinFiles::open(const AString& file)
{
	if (auto c = instance().mBuffers.contains(file))
	{
		return _new<ByteBufferInputStream>(c->second);
	}
	return nullptr;
}

void BuiltinFiles::load(const unsigned char* data, size_t size) {
    printf("adssa");
}
