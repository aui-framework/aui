// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "error.h"
#include "message.h"

namespace DBus {

Error::Error() {
}

Error::Error( const char* name, const char* message ) {
    if( name ) {
        m_name = std::string( name );
    }

    if( message != nullptr ) {
        m_message = std::string( message );
    }
}

Error::Error( const char* name, std::string message ) {
    m_name = std::string( name );
    m_message = std::string( message );
}

Error::Error( std::string name, std::string message ) {
    m_name = std::string( name );
    m_message = std::string( message );
}

Error::~Error( ) noexcept {
}

std::string Error::name() const {
    return m_name;
}

std::string Error::message() const {
    return m_message;
}

const char* Error::what() const noexcept {
    return m_message.c_str();
}

}


