// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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