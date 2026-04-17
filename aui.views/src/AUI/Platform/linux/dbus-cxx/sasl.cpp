// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "sasl.h"

#include "dbus-cxx-private.h"

#include <cstring>
#include <ostream>
#include <poll.h>
#include <regex>
#include <sstream>
#include <unistd.h>

using DBus::priv::SASL;

class SASL::priv_data {
public:
    priv_data( int fd, bool negotiateFDPassing ) :
        m_fd( fd ),
        m_negotiateFDpassing( negotiateFDPassing )
    {}

    int m_fd;
    bool m_negotiateFDpassing;
};

static const std::regex OK_REGEX( "OK ([a-z0-9]*)" );
static const std::regex DATA_REGEX( "DATA ([a-z0-9]*)" );
static const std::regex ERROR_REGEX( "ERROR(.*)" );
static const std::regex AGREE_UNIX_FD_REGEX( "AGREE_UNIX_FD" );
static const std::regex REJECTED_REGEX( "REJECTED (.*)" );
static const char* LOGGER_NAME = "DBus.priv.SASL";

static int hexchar2int( char c ) {
    if( c >= '0' && c <= '9' ) {
        return c - 48;
    }

    if( c >= 'a' && c <= 'f' ) {
        return c - 97;
    }

    return 0;
}

SASL::SASL( int fd, bool negotiateFDPassing ) :
    m_priv( std::make_unique<priv_data>( fd, negotiateFDPassing ) ) {

}

SASL::~SASL() {}

std::tuple<bool, bool, std::vector<uint8_t>> SASL::authenticate() {
    bool success = false;
    bool negotiatedFD = false;
    std::vector<uint8_t> serverGUID;
    uid_t uid = getuid();
    std::string line;
    std::smatch regex_match;

    write_data_with_newline( "AUTH EXTERNAL " + encode_as_hex( uid ) );

    line = read_data();
    if( line.length() == 0 ){
        goto out;
    }

    if( std::regex_search( line, regex_match, OK_REGEX ) ) {
        serverGUID = hex_to_vector( regex_match[ 1 ] );
    } else if( std::regex_search( line, regex_match, ERROR_REGEX ) ) {
        SIMPLELOGGER_DEBUG( "DBus.priv.SASL", "Unable to authenticate: "
            + regex_match[ 1 ].str() );
        goto out;
    } else if( std::regex_search( line, regex_match, REJECTED_REGEX ) ) {
        SIMPLELOGGER_DEBUG( "DBus.priv.SASL", "Rejected authentication, available modes: "
            + regex_match[ 1 ].str() );
        goto out;
    } else {
        // Unknown command, return an error to the server
        write_data_with_newline( "ERROR Unrecognized response" );
        goto out;
    }

    if( m_priv->m_negotiateFDpassing ) {
        write_data_with_newline( "NEGOTIATE_UNIX_FD" );
        line = read_data();

        if( line.length() == 0 ){
            goto out;
        }

        if( std::regex_search( line, regex_match, AGREE_UNIX_FD_REGEX ) ) {
            negotiatedFD = true;
        }

        if( std::regex_search( line, regex_match, ERROR_REGEX ) ) {
            SIMPLELOGGER_DEBUG( "DBus.priv.SASL", "Unable to negotiate FD passing: "
                + regex_match[ 1 ].str() );
        }
    }

    write_data_with_newline( "BEGIN" );

    success = true;

out:

    return std::make_tuple( success, negotiatedFD, serverGUID );
}

int SASL::write_data_with_newline( std::string data ) {
    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Sending command: " + data );
    data += "\r\n";
    return ::write( m_priv->m_fd, data.c_str(), data.length() );
}

std::string SASL::read_data() {
    std::string line_read;
    char dataBuffer[ 512 ];
    ssize_t bytesRead;

    pollfd pollfd;
    pollfd.fd = m_priv->m_fd;
    pollfd.events = POLLIN;

    if( poll( &pollfd, 1, -1 ) < 0 ) {
        std::string errmsg = strerror( errno );
        SIMPLELOGGER_ERROR( "DBus.priv.SASL", "Unable to poll for response from daemon: " + errmsg );
        return line_read;
    }

    // TODO Note that we are making an assumption here that we don't
    // have a short read.  This may need to be fixed in the future.
    bytesRead = ::read( m_priv->m_fd, &dataBuffer, 512 );
    if( bytesRead < 0 ){
        SIMPLELOGGER_ERROR( LOGGER_NAME, "Unable to read SASL data!" );
        return std::string();
    }
    line_read = std::string( dataBuffer, bytesRead );

    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Received response: " + line_read );

    return line_read;
}

std::string SASL::encode_as_hex( int num ) {
    std::ostringstream out;
    std::ostringstream numString;
    std::string finalNumString;

    numString << num;

    finalNumString = numString.str();
    out << std::hex;

    for( const char& s : finalNumString ) {
        out << std::hex << ( int )s;
    }

    return out.str();

}

std::vector<uint8_t> SASL::hex_to_vector( std::string hexData ) {
    std::vector<uint8_t> retval;

    if( hexData.size() % 2 != 0 ) {
        return retval;
    }

    for( std::string::const_iterator it = hexData.cbegin();
        it != hexData.cend(); ) {
        int first = hexchar2int( *it );
        it++;
        int second = hexchar2int( *it );
        it++;

        retval.push_back( first << 4 | second );
    }

    return retval;
}
