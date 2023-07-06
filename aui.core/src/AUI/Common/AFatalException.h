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
 * @brief An exception that thrown when access violation (segfault), or abort occurs.
 * @ingroup core
 * @details
 * Your application may handle AFatalException and continue normal execution.
 */
class API_AUI_CORE AFatalException: public AException {
public:
    using Handler = std::function<void(AFatalException*)>;

    explicit AFatalException(void* address, std::string_view signalName):
        AException("{} at address {}"_format(signalName, address)), mAddress(address) {
        if (handler())
            handler()(this);
    }

    void* getAddress() const {
        return mAddress;
    }

    static void setGlobalHandler(Handler globalHandler) {
        handler() = std::move(globalHandler);
    }

private:
    void* mAddress;

    static Handler& handler();
};