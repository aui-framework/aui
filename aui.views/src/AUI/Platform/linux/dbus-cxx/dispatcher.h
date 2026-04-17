// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009,2010 by Rick L. Vinyard, Jr.             *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/

#include <string>
#include <memory>
#include "enums.h"

#ifndef DBUSCXX_DISPATCHER_H
#define DBUSCXX_DISPATCHER_H

namespace DBus {
class Connection;

/**
 * A Dispatcher handles all of the reading/writing that a Connection
 * needs in order to work correctly.  Logically, there should only be
 * one Dispatcher per application for the lifetime of that application.
 */
class Dispatcher {
public:
    virtual ~Dispatcher();

    /** @name Managing Connections */
    //@{
    virtual std::shared_ptr<Connection> create_connection( BusType type ) = 0;

    virtual std::shared_ptr<Connection> create_connection( std::string address ) = 0;

    virtual bool add_connection( std::shared_ptr<Connection> connection ) = 0;

    //@}
};

}

#endif
