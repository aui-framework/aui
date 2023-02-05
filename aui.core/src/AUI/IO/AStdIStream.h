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

#include <istream>
#include "IInputStream.h"


/**
 * @brief std::istream implementation wrapping AUI's IInputStream.
 * @ingroup io
 */
class API_AUI_CORE AStdIStream: public std::istream {
public:
    class StreamBuf: public std::streambuf  {
    public:
        StreamBuf(_<IInputStream> is);

        virtual ~StreamBuf();

    protected:
        int_type underflow() override;

    private:
        _<IInputStream> mIs;

        char mBuffer[0x1000]{};
    };

    AStdIStream(_<IInputStream> is): std::istream(new StreamBuf(std::move(is))) {

    }
    ~AStdIStream() {
        delete rdbuf();
    }
};