#include "BuiltinFiles.h"

#include "LZ.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/ByteBufferInputStream.h"

void BuiltinFiles::loadBuffer(AByteBuffer& data)
{
	AByteBuffer unpacked;
	LZ::decompress(data, unpacked);

	while (unpacked.getAvailable())
	{
		std::string file;
		unpacked >> file;

		uint32_t s;
		unpacked >> s;

		auto b = _new<AByteBuffer>();
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
	    c->second->setCurrentPos(0);
		return _new<ByteBufferInputStream>(c->second);
	}
	return nullptr;
}

void BuiltinFiles::load(const unsigned char* data, size_t size) {
    AByteBuffer b(data, size);
    instance().loadBuffer(b);
}
