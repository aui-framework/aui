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

#include "AStdIStream.h"

AStdIStream::StreamBuf::StreamBuf(_<IInputStream> is) : mIs(std::move(is)) {
    setg(0, 0, 0);
    setp(mBuffer, mBuffer + std::size(mBuffer));
}

AStdIStream::StreamBuf::~StreamBuf() {

}

int AStdIStream::StreamBuf::underflow() {
    auto r = mIs->read(mBuffer, std::size(mBuffer));
    if (r == 0) return std::basic_ios<char>::traits_type::eof();
    setg(mBuffer, mBuffer, mBuffer + r);
    return std::basic_ios<char>::traits_type::to_int_type(*gptr());
}
