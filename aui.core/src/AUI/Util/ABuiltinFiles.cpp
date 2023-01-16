// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "ABuiltinFiles.h"

#include "LZ.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/AByteBufferInputStream.h"

void ABuiltinFiles::loadBuffer(AByteBuffer& data)
{
	AByteBuffer unpacked;
	LZ::decompress(data, unpacked);
    AByteBufferInputStream bis(unpacked);
	while (bis.available())
	{
		std::string file;
        AByteBuffer b;
        bis >> aui::serialize_sized(file);
        bis >> aui::serialize_sized(b);
        inst().mBuffers[AString(file)] = std::move(b);
	}
}

_<IInputStream> ABuiltinFiles::open(const AString& file)
{
	if (auto c = inst().mBuffers.contains(file))
	{
		return _new<AByteBufferInputStream>(c->second);
	}
	return nullptr;
}

AOptional<AByteBufferView> ABuiltinFiles::getBuffer(const AString& file) {
    if (auto c = inst().mBuffers.contains(file))
    {
        return AByteBufferView(c->second);
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
