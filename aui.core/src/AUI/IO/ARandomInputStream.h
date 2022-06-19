#pragma once


#include <random>
#include "IInputStream.h"

/**
 * @brief An input stream that generates random bytes.
 * @ingroup core
 */
class API_AUI_CORE ARandomInputStream: public IInputStream {
private:
    std::mt19937 mRandomEngine;

public:
    explicit ARandomInputStream(size_t seed): mRandomEngine(seed) {}
    ~ARandomInputStream() override = default;

    size_t read(char* dst, size_t size) override;
};


