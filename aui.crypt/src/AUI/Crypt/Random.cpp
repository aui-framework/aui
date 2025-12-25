/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 25.09.2020.
//

#include "Random.h"

#include <random>
#include <chrono>

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
