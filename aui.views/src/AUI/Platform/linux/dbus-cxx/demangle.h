// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2019 by Robert Middleton.                               *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/

#ifndef DBUSCXX_DEMANGLE_H
#define DBUSCXX_DEMANGLE_H

#include <string>
#include <typeinfo>

#if DBUS_CXX_HAS_CXXABI_H
    #include <cxxabi.h>
#endif

namespace DBus {
/**
 * demangle the given type.  Used for debugging purposes.
 */
template <typename T>
std::string demangle() {
#if DBUS_CXX_HAS_CXA_DEMANGLE
    int status;
    T arg1 {};
    char* demangled = abi::__cxa_demangle( typeid( arg1 ).name(), nullptr, nullptr, &status );
    std::string arg1_name( demangled );
    free( demangled );

    if( status < 0 ) {
        arg1_name = typeid( T ).name();
    }

#else
    std::string arg1_name = typeid( T ).name();
#endif

    return arg1_name;
}

} /* namespace DBus */

#endif
