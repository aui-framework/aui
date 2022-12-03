// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
#include "AUI/Common/AString.h"
#include "IInputStream.h"
#include "IOutputStream.h"

/**
 * @brief A buffer input/output stream based on std::string.
 * @ingroup io
 */
class API_AUI_CORE AStringStream: public IInputStream, public IOutputStream
{
private:
	std::string mString;
	size_t mReadPos = 0;

public:
    AStringStream();
	explicit AStringStream(const AString& string);
	virtual ~AStringStream() = default;

	void seekRead(size_t position);

	size_t read(char* dst, size_t size) override;
    void write(const char *src, size_t size) override;

    [[nodiscard]]
    const std::string& str() const noexcept {
        return mString;
    }
};
