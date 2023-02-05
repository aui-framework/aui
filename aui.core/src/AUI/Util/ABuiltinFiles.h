// AUI Framework - Declarative UI toolkit for modern C++20
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

#pragma once

#include "AUI/Core.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include <optional>

class AString;

class API_AUI_CORE ABuiltinFiles
{
private:
	AMap<AString, AByteBuffer> mBuffers;

	static ABuiltinFiles& inst();
	ABuiltinFiles() = default;

public:
	static void loadBuffer(AByteBuffer& data);
	static void load(const unsigned char* data, size_t size);
	static _<IInputStream> open(const AString& file);
    static AOptional<AByteBufferView> getBuffer(const AString& file);
};
