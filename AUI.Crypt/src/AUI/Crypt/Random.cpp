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
    bb->put(garbage, GARBAGE_SIZE);
    bb << garbage;
    delete[] garbage;

    auto seed2 = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    std::default_random_engine re(seed1 ^ seed2);

    for (size_t i = 0; i < 10; ++i) {
        bb << std::uniform_int_distribution<size_t>()(re);
    }
    auto seed3 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    bb << seed3 << seed1 << seed2;
    bb->setCurrentPos(0);

    return bb;
}
