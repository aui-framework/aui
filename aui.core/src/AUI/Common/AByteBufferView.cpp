// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

//
// Created by Alex2772 on 4/18/2022.
//

#include <AUI/IO/AByteBufferInputStream.h>
#include "AByteBufferView.h"

AString AByteBufferView::toHexString() const {
    AString result;
    result.reserve(size() * 2 + 10);
    char buf[8];

    for (size_t i = 0; i < size(); ++i) {
        result += std::string_view(buf, std::distance(std::begin(buf), fmt::format_to(std::begin(buf), "{:02x}", (std::uint8_t)mBuffer[i])));
    }
    return result;
}

_<IInputStream> AByteBufferView::toStream() const {
    class ByteBufferStream: public IInputStream {
    private:
        AByteBuffer mData;
        AByteBufferInputStream mIs;

    public:
        ByteBufferStream(AByteBufferView view): mData(view), mIs(mData) {

        }

        size_t read(char* dst, size_t size) override {
            return mIs.read(dst, size);
        }
    };

    return _new<ByteBufferStream>(*this);
}
