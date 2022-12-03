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


#include <random>
#include "IInputStream.h"

/**
 * @brief An input stream that generates random bytes.
 * @ingroup io
 */
class API_AUI_CORE ARandomInputStream: public IInputStream {
private:
    std::mt19937 mRandomEngine;

public:
    explicit ARandomInputStream(size_t seed): mRandomEngine(seed) {}
    ~ARandomInputStream() override = default;

    size_t read(char* dst, size_t size) override;
};


