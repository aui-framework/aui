#pragma once


#include <random>
#include "IInputStream.h"

class API_AUI_CORE ARandomInputStream: public IInputStream {
private:
    std::mt19937 mRandomEngine;

public:
    explicit ARandomInputStream(size_t seed): mRandomEngine(seed) {}
    ~ARandomInputStream() override = default;

    int read(char* dst, int size) override;
};


