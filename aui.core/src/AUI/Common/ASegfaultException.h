//
// Created by alex2772 on 1/14/22.
//

#pragma once

#include "AException.h"

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