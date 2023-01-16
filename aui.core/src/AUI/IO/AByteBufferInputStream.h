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

#pragma once
#include "IInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/SharedPtr.h"


/**
 * @brief Converts a AByteBuffer to an IInputStream.
 * @ingroup io
 */
class API_AUI_CORE AByteBufferInputStream final: public IInputStream
{
private:
    const char* mCurrent;
    const char* mEnd;


public:
	AByteBufferInputStream(AByteBufferView buffer)
		: mCurrent(buffer.data()), mEnd(buffer.data() + buffer.size())
	{
	}

	size_t read(char* dst, size_t size) override;

    size_t available() const {
        return mEnd - mCurrent;
    }
};
