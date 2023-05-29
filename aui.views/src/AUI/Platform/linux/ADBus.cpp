//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2772 on 5/28/23.
//

#include "ADBus.h"
#include "AUI/Util/ARaiiHelper.h"

ADBus& ADBus::inst() {
    static ADBus d;
    return d;
}


template<aui::invocable Callback>
void ADBus::throwExceptionOnError(Callback&& callback) {
    callback();
    if (dbus_error_is_set(mError.ptr())) {
        throw ADBus::Exception(mError->name);
    }
}

ADBus::ADBus() {
    dbus_bus_get(DBUS_BUS_SESSION, mError.ptr());
    throwExceptionOnError([&] { dbus_error_init(mError.ptr()); });
    throwExceptionOnError([&] { mConnection = dbus_bus_get(DBUS_BUS_SESSION, mError.ptr()); });
}

ADBus::~ADBus() {
    throwExceptionOnError([&] { dbus_error_free(mError.ptr()); });
}

