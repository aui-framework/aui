// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "validator.h"

using DBus::Validator;

static bool is_allowable_character( char c ) {
    if( c >= 'A' && c <= 'Z' ) {
        return true;
    }

    if( c >= 'a' && c <= 'z' ) {
        return true;
    }

    if( c >= '0' && c <= '9' ) {
        return true;
    }

    if( c == '_' ) {
        return true;
    }

    return false;
}


bool Validator::validate_bus_name( std::string busname ) {
    char previousChar = '\0';
    int numElements = 1;
    std::string::iterator it = busname.begin();
    bool isUnique = false;

    if( busname.size() > 255 || busname.empty() ) {
        return false;
    }

    if( *it == '.' ) {
        // A period is not allowed as the first character
        return false;
    }

    if( *it == ':' ) {
        // A colon is allowed as the first character
        it++;
        isUnique = true;
    }

    for( ; it < busname.end(); it++ ) {
        if( *it != '.' && !is_allowable_character( *it ) ) { return false; }

        if( previousChar == '.' && is_allowable_character( *it ) ) {
            if( !isUnique && std::isdigit( *it ) ) {
                return false;
            }

            numElements++;
        }

        previousChar = *it;
    }

    return numElements >= 2;
}

bool Validator::validate_interface_name( std::string interfacename ) {
    char previousChar = '\0';
    int numElements = 1;
    std::string::iterator it = interfacename.begin();

    if( interfacename.size() > 255 || interfacename.empty() ) {
        return false;
    }

    if( *it == '.' ) {
        // A period is not allowed as the first character
        return false;
    }

    for( ; it < interfacename.end(); it++ ) {
        if( *it != '.' && !is_allowable_character( *it ) ) { return false; }

        if( previousChar == '.' && is_allowable_character( *it ) ) {
            if( std::isdigit( *it ) ) {
                return false;
            }

            numElements++;
        }

        previousChar = *it;
    }

    return numElements >= 2;
}

bool Validator::validate_member_name( std::string name ) {
    if( name.size() > 255 || name.empty() ) {
        return false;
    }

    // Member names must not begin with a digit
    if( name.at( 0 ) < '0' ||
        name.at( 0 ) > '9' ) {
        return false;
    }

    for( const char& c : name ) {
        if( !is_allowable_character( c ) ) {
            return false;
        }
    }

    return true;
}

bool Validator::validate_error_name( std::string errorname ) {
    return validate_interface_name( errorname );
}

bool Validator::message_is_small_enough( const std::vector<uint8_t>* data ) {
    return data->size() < maximum_message_size();
}

