//
// Created by alex2772 on 1/14/22.
//

#pragma once

#include "AException.h"

/**
 * @brief An exception that thrown when access violation (segfault) occurs.
 * @ingroup core
 * @details
 * Your application may handle ASegfaultException and continue normal execution.
 */
class ASegfaultException: public AException {
private:
    void* mAddress;

public:
    ASegfaultException(void* address) : mAddress(address) {}

    AString getMessage() const noexcept override {
        char buf[128];
        std::sprintf(buf, "segmentation fault at address %p", mAddress);
        return buf;
    }

    void* getAddress() const {
        return mAddress;
    }
};