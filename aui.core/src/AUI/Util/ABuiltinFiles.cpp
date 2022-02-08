/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "ABuiltinFiles.h"

#include "LZ.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/AByteBufferInputStream.h"

void ABuiltinFiles::loadBuffer(AByteBuffer& data)
{
	AByteBuffer unpacked;
	LZ::decompress(data, unpacked);

	while (unpacked.availableToRead())
	{
		std::string file;
        AByteBuffer b;
        unpacked >> file;
		unpacked >> b;
        inst().mBuffers[AString(file)] = std::move(b);
	}
}

_<IInputStream> ABuiltinFiles::open(const AString& file)
{
	if (auto c = inst().mBuffers.contains(file))
	{
	    c->second.setCurrentPos(0);
		return _new<AByteBufferInputStream>(c->second);
	}
	return nullptr;
}

std::optional<AByteBufferRef> ABuiltinFiles::getBuffer(const AString& file) {
    if (auto c = inst().mBuffers.contains(file))
    {
        return c->second.ref();
    }
    return std::nullopt;
}

ABuiltinFiles& ABuiltinFiles::inst() {
    static ABuiltinFiles f;
    return f;
}

void ABuiltinFiles::load(const unsigned char* data, size_t size) {
    AByteBuffer b(data, size);
    inst().loadBuffer(b);
}
