// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_SASL_H
#define DBUSCXX_SASL_H

#include <memory>
#include <stdint.h>
#include <string>
#include <tuple>
#include <vector>

namespace DBus {
namespace priv {

/**
 * Implements the authentication routines for connection to the bus
 */
class SASL {
public:
    /**
     * Create a new SASL authenticator
     *
     * @param fd The FD to authenticate on
     * @param negotiateFDPassing True if we should try to negotiate
     * FD passing, false otherwise
     */
    SASL( int fd, bool negotiateFDPassing );

    ~SASL();

    /**
     * Perform the authentication with the server.
     *
     * @return A tuple containing the following:
     * - bool Success of authentication
     * - bool If this supports FD passing
     * - vector The GUID of the server
     */
    std::tuple<bool, bool, std::vector<uint8_t>> authenticate();

private:
    int write_data_with_newline( std::string data );
    std::string read_data();
    std::string encode_as_hex( int num );
    std::vector<uint8_t> hex_to_vector( std::string hexData );

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;
};

} /* namespace priv */
} /* namespace DBus */

#endif /* DBUSCXX_SASL_H */
