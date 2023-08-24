#pragma once

#include <cstddef>

class ISoundSource {
public:
    virtual size_t requestSoundData(char* dst, size_t size) = 0;
};

///TODO seems like this interface is useless
