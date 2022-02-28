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
