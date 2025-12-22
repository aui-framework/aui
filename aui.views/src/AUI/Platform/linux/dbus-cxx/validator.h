// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_VALIDATOR_H
#define DBUSCXX_VALIDATOR_H

#include <string>
#include <vector>
#include <stdint.h>

namespace DBus {

/**
 * Contains various static routines for validating and/or sanitizing data.
 */
class Validator {
private:
    Validator();

public:
    /**
     * Validate a bus name.  According to the DBus specification:
     *
     * Bus names that start with a colon (':') character are unique connection names.
     * Other bus names are called well-known bus names.
     *
     * -Bus names are composed of 1 or more elements separated by a period ('.') character.
     * All elements must contain at least one character.
     * - Each element must only contain the ASCII characters "[A-Z][a-z][0-9]_-", with "-"
     * discouraged in new bus names. Only elements that are part of a unique connection
     * name may begin with a digit, elements in other bus names must not begin with a digit.
     * - Bus names must contain at least one '.' (period) character (and thus at least two elements).
     * - Bus names must not begin with a '.' (period) character.
     * - Bus names must not exceed the maximum name length(255).
     *
     * Note that this method will reject technically-correct interface names that have '-' in them,
     * as this is discouraged.
     *
     * @param name The name to validate
     * @return
     */
    static bool validate_bus_name( std::string name );

    /**
     * Validate an interface name.  According to the DBus specification:
     *
     * Interface names are composed of 2 or more elements separated by a period ('.') character.
     * All elements must contain at least one character.
     *
     * - Each element must only contain the ASCII characters "[A-Z][a-z][0-9]_" and must not begin with a digit.
     * - Interface names must not exceed the maximum name length.
     *
     * @param name The name to validate
     * @return
     */
    static bool validate_interface_name( std::string name );

    /**
     * Validate a member name.  According to the DBus specification:
     *
     * - Must only contain the ASCII characters "[A-Z][a-z][0-9]_" and may not begin with a digit.
     * - Must not contain the '.' (period) character.
     * - Must not exceed the maximum name length.
     * - Must be at least 1 byte in length.
     *
     * @param name
     * @return
     */
    static bool validate_member_name( std::string name );

    /**
     * Validate an error name.  See validate_interface_name for specifications.
     *
     * @param name
     * @return
     */
    static bool validate_error_name( std::string name );

    /**
     * Checks to make sure that the size of the message(after serialization) is lower
     * than 2^27
     *
     * @param data
     * @return
     */
    static bool message_is_small_enough( const std::vector<uint8_t>* data );

    /**
     * Return the maximum array size
     * @return
     */
    static constexpr uint32_t maximum_array_size() { return 0x01 << 26; }

    static constexpr uint32_t maximum_message_size() { return ( 0x01 << 27 ); }

    static constexpr uint32_t maximum_message_depth() { return 64; }

};

} /* namespace DBus */

#endif /* DBUSCXX_VALIDATOR_H */
