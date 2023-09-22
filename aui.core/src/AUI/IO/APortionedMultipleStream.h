#pragma once

#include "IInputStream.h"

/**
 * @brief //TODO
 */
class APortionedMultipleStream : public IInputStream {
public:
    void append(_<IInputStream> stream) {
        mDeque << std::move(stream);
    }

    APortionedMultipleStream& operator<<(_<IInputStream> stream) {
        mDeque << std::move(stream);
        return *this;
    }

    size_t read(char* dst, size_t size) override;

private:
    ADeque<_<IInputStream>> mDeque;
};
