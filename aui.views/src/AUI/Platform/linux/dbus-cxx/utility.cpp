// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "utility.h"
#include <stdio.h>
#include <iostream>
#include <mutex>
#include <new>
#include <thread>
#include "connection.h"
#include "dbus-cxx-private.h"
#include "error.h"
#include "message.h"
#include "simplelogger_defs.h"
#include <locale>
#include <chrono>

#include <poll.h>

/* Extern function for logging in headers */
simplelogger_log_function dbuscxx_log_function = nullptr;

[[gnu::constructor]]
static void  configure_endianess(){
    char* env_var = getenv("DBUSCXX_ENDIANESS");
    if(env_var == nullptr){
        return;
    }

    if(env_var[0] == 0){
        return;
    }

    if(env_var[0] == 'B'){
        DBus::set_default_endianess(DBus::Endianess::Big);
    }else if(env_var[0] == 'l'){
        DBus::set_default_endianess(DBus::Endianess::Little);
    }
}

namespace DBus {

static enum SL_LogLevel log_level = SL_INFO;
static Endianess lib_default_endianess = Endianess::Big;

void set_logging_function( simplelogger_log_function function ) {
    dbuscxx_log_function = function;
}

void log_std_err( const char* logger_name, const struct SL_LogLocation* location,
    const enum SL_LogLevel level,
    const char* log_string ) {
    if( level < log_level ) { return; }

    char buffer[ 4096 ];
    const char* stringLevel;
    std::thread::id this_id = std::this_thread::get_id();
    std::hash<std::thread::id> hasher;

    SL_LOGLEVEL_TO_STRING( stringLevel, level );

    snprintf( buffer, 4096, "0x%08lX %s [%s] - %s(%s:%d)", hasher(this_id), logger_name, stringLevel, log_string,
        location->file,
        location->line_number );
    std::cerr << buffer << std::endl;
}

void set_log_level( const enum SL_LogLevel level ) {
    log_level = level;
}

void hexdump( const std::vector<uint8_t>* vec, std::ostream* stream ) {
    // Original C code: https://stackoverflow.com/a/29865/624483
    hexdump( vec->data(), vec->size(), stream );
}

void hexdump( const uint8_t* vec, uint32_t len, std::ostream* stream ) {
    // Original C code: https://stackoverflow.com/a/29865/624483
    char line_buffer[ 12 ];

    for( uint32_t i = 0; i < len; i += 16 ) {
        snprintf( line_buffer, 12, "%06x: ", i );
        *stream << line_buffer;

        for( int j = 0; j < 16; j++ ) {
            if( j == 8 ) { *stream << "  "; }

            if( ( i + j ) < len ) {
                snprintf( line_buffer, 12, "%02x ", vec[ i + j ] );
                *stream << line_buffer;
            } else {
                *stream << "   ";
            }
        }

        *stream << "  ";

        for( int j = 0; j < 16; j++ ) {
            if( j == 8 ) { *stream << "  "; }

            if( ( i + j ) < len ) {
                snprintf( line_buffer, 12, "%c", std::isprint( vec[i + j] ) ? vec[i + j] : '.' );
                *stream << line_buffer;
            }
        }

        *stream << std::endl;
    }
}

std::tuple<bool, int, std::vector<int>, std::chrono::milliseconds> priv::wait_for_fd_activity( std::vector<int> fds, int timeout_ms ) {
    std::vector<pollfd> toListen;
    bool timeout;
    int poll_ret;
    std::chrono::milliseconds ms_waited;
    std::vector<int> fdsToRead;

    toListen.reserve( fds.size() );

    for( int fd : fds ) {
        struct pollfd pollfd;
        pollfd.fd = fd;
        pollfd.events = POLLIN;
        pollfd.revents = 0;
        toListen.push_back( pollfd );
    }

    std::chrono::time_point start = std::chrono::steady_clock::now();

    do {
        poll_ret = ::poll( toListen.data(), toListen.size(), timeout_ms );

        if( poll_ret >= 0 ) {
            timeout = poll_ret == 0 ? true : false;
            ms_waited = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start
                );

            for( pollfd pollentry : toListen ) {
                if( pollentry.revents & POLLIN ) {
                    fdsToRead.push_back( pollentry.fd );
                }
            }

            break;
        }

        if( poll_ret < 0 && errno == EINTR ) {
            continue;
        }
    } while( true );

    return std::make_tuple( timeout, poll_ret, fdsToRead, ms_waited );
}

void set_default_endianess(DBus::Endianess endianess){
    lib_default_endianess = endianess;
}

DBus::Endianess default_endianess(){
    return lib_default_endianess;
}

} /* namespace DBus */



