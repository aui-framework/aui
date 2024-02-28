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
// Created by alex2 on 25.09.2020.
//

#include <random>
#include "Random.h"

_<AByteBuffer> ACrypto::safeRandom() {
    auto bb = _new<AByteBuffer>();

    auto seed1 = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    const size_t GARBAGE_SIZE = 0x1000;
    char* garbage = new char[GARBAGE_SIZE];
    bb->write(garbage, GARBAGE_SIZE);
    bb->write(reinterpret_cast<const char*>(&garbage), sizeof(&garbage));
    delete[] garbage;

    auto seed2 = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    std::default_random_engine re(seed1 ^ seed2);

    for (size_t i = 0; i < 10; ++i) {
        bb << std::uniform_int_distribution<size_t>()(re);
    }
    auto seed3 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    bb << seed3 << seed1 << seed2;

    return bb;
}
