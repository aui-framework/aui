// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_STANDALONE_DISPATCHER
#define DBUSCXX_STANDALONE_DISPATCHER

#include "dispatcher.h"

namespace DBus {

class Connection;

/**
 * The StandaloneDispatcher creates a new thread that handles all of the
 * reading and writing to the bus.
 *
 * One dispatcher can handle multiple connections.
 */
class StandaloneDispatcher : public Dispatcher {
private:

    StandaloneDispatcher( bool is_running = true );

public:

    static std::shared_ptr<StandaloneDispatcher> create( bool is_running = true );

    ~StandaloneDispatcher();

    /** @name Managing Connections */
    //@{
    std::shared_ptr<Connection> create_connection( BusType type );

    std::shared_ptr<Connection> create_connection( std::string address );

    bool add_connection( std::shared_ptr<Connection> connection );

    //@}

    bool start();

    bool stop();

    bool is_running();

private:

    void dispatch_thread_main();

    void wakeup_thread();

    /**
     * Dispatch all of our connections
     */
    void dispatch_connections();

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;
};

} /* namespace DBus */

#endif /* DBUSCXX_STANDALONE_DISPATCHER */
