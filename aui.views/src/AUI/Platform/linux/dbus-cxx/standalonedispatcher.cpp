// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/connection.h>
#include <AUI/Platform/linux/dbus-cxx/dbus-cxx-private.h>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <deque>
#include <utility>
#include <string.h>

#include "standalonedispatcher.h"

#if defined( _WIN32 ) && defined( connect )
    #undef connect
#endif

using DBus::StandaloneDispatcher;

static const char* LOGGER_NAME = "DBus.StandaloneDispatcher";

class StandaloneDispatcher::priv_data {
public:
    priv_data() :
        m_running( false ),
        m_dispatch_loop_limit( 0 ) {

    }

    std::vector<std::shared_ptr<Connection>> m_connections;
    volatile bool m_running;
    std::thread m_dispatch_thread;
    /* socketpair for telling the thread to process data */
    int process_fd[ 2 ];
    /**
     * This is the maximum number of dispatches that will occur for a
     * connection in one iteration of the dispatch thread.
     *
     * If set to 0, a particular connection will continue to dispatch
     * as long as its status remains DISPATCH_DATA_REMAINS.
     */
    unsigned int m_dispatch_loop_limit;

};

StandaloneDispatcher::StandaloneDispatcher( bool is_running ) {
    m_priv = std::make_unique<priv_data>();

    if( socketpair( AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, m_priv->process_fd ) < 0 ) {
        SIMPLELOGGER_ERROR( LOGGER_NAME, "error creating socket pair" );
        throw ErrorDispatcherInitFailed();
    }

    if( is_running ) { this->start(); }
}

std::shared_ptr<StandaloneDispatcher> StandaloneDispatcher::create( bool is_running ) {
    return std::shared_ptr<StandaloneDispatcher>( new StandaloneDispatcher( is_running ) );
}

StandaloneDispatcher::~StandaloneDispatcher() {
    this->stop();
}

std::shared_ptr<DBus::Connection> StandaloneDispatcher::create_connection( std::string address ) {
    std::shared_ptr<Connection> conn = Connection::create( address );
    conn->bus_register();

    if( this->add_connection( conn ) ) { return conn; }

    return std::shared_ptr<Connection>();
}

std::shared_ptr<DBus::Connection> StandaloneDispatcher::create_connection( BusType type ) {
    std::shared_ptr<Connection> conn = Connection::create( type );
    conn->bus_register();

    if( this->add_connection( conn ) ) { return conn; }

    return std::shared_ptr<Connection>();
}

bool StandaloneDispatcher::add_connection( std::shared_ptr<Connection> connection ) {
    if( !connection || !connection->is_valid() ) { return false; }

    connection->set_dispatching_thread( m_priv->m_dispatch_thread.get_id() );
    connection->signal_needs_dispatch().connect( sigc::mem_fun( *this, &StandaloneDispatcher::wakeup_thread ) );
    m_priv->m_connections.push_back( connection );
    wakeup_thread();

    return true;
}

bool StandaloneDispatcher::start() {
    if( m_priv->m_running ) { return false; }

    m_priv->m_running = true;

    m_priv->m_dispatch_thread = std::thread( &StandaloneDispatcher::dispatch_thread_main, this );

    return true;
}

bool StandaloneDispatcher::stop() {
    if( !m_priv->m_running ) { return false; }

    m_priv->m_running = false;

    wakeup_thread();

    if( m_priv->m_dispatch_thread.joinable() ) {
        m_priv->m_dispatch_thread.join();
    }

    return true;
}

bool StandaloneDispatcher::is_running() {
    return m_priv->m_running;
}

void StandaloneDispatcher::dispatch_thread_main() {
    std::vector<int> fds;

    for( std::shared_ptr<Connection> conn : m_priv->m_connections ) {
        conn->set_dispatching_thread( std::this_thread::get_id() );
    }

    while( m_priv->m_running ) {
        fds.clear();
        fds.push_back( m_priv->process_fd[ 1 ] );

        for( std::shared_ptr<Connection> conn : m_priv->m_connections ) {
            if( !conn->is_registered() ) {
                conn->bus_register();
            }

            fds.push_back( conn->unix_fd() );
        }

        std::tuple<bool, int, std::vector<int>, std::chrono::milliseconds> fdResponse =
            DBus::priv::wait_for_fd_activity( fds, -1 );
        std::vector<int> fdsToRead = std::get<2>( fdResponse );

        if( !fdsToRead.empty() && fdsToRead[ 0 ] == m_priv->process_fd[ 1 ] ) {
            char discard;
            if( read( m_priv->process_fd[ 1 ], &discard, sizeof( char ) ) < 0 ){
                SIMPLELOGGER_DEBUG( LOGGER_NAME, "Failure reading from dispatch thread process_fd: "
                                    << strerror( errno ) );
            }
        }

        dispatch_connections();
    }
}

void StandaloneDispatcher::dispatch_connections() {
    uint32_t loop_limit = m_priv->m_dispatch_loop_limit;

    if( loop_limit == 0 ) {
        loop_limit = UINT32_MAX;
    }

    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Dispatching connections" );

    for( std::shared_ptr<Connection> conn : m_priv->m_connections ) {
        for( uint32_t x = 0; x < loop_limit; x++ ) {
            DispatchStatus stat = conn->dispatch();

            if( stat == DispatchStatus::COMPLETE ) {
                break;
            }
        }

        if( conn->dispatch_status() != DispatchStatus::COMPLETE ) {
            wakeup_thread();
        }
    }

    SIMPLELOGGER_DEBUG( LOGGER_NAME, "done dispatching" );
}

void StandaloneDispatcher::wakeup_thread() {
    char to_write = '0';

    if( write( m_priv->process_fd[ 0 ], &to_write, sizeof( char ) ) < 0 ) {
        SIMPLELOGGER_ERROR( LOGGER_NAME, "Can't write to socketpair?!" );
    }
}
