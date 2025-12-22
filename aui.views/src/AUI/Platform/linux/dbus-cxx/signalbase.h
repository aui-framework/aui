// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/path.h>
#include <stddef.h>
#include <memory>
#include <string>

#ifndef DBUSCXX_SIGNALBASE_H
#define DBUSCXX_SIGNALBASE_H

namespace DBus {
class Connection;
class Message;

/**
 * @defgroup signals Signals
 * Classes for interacting with signals(sending and receiving)
 */

/**
 * Base class of all signals(proxy and adapter).  This is required to interact with signals
 * in a generic manner, as the actual signal implementation class is templated.
 *
 * @ingroup signals
 * @ingroup local
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class SignalBase {
protected:

    SignalBase( const std::string& path, const std::string& interface_name, const std::string& name );

    SignalBase( const std::string& interface_name, const std::string& name );

    SignalBase( std::weak_ptr<Connection> connection, const std::string& path, const std::string& interface_name, const std::string& name );

    SignalBase( std::weak_ptr<Connection> connection, const std::string& interface_name, const std::string& name );

public:
    virtual ~SignalBase();

    std::shared_ptr<Connection> connection();

    void set_connection( std::weak_ptr<Connection> connection );

    const std::string& sender() const;

    void set_sender( const std::string& s );

    const std::string& interface_name() const;

    void set_interface( const std::string& i );

    const std::string& name() const;

    void set_name( const std::string& n );

    const Path& path() const;

    void set_path( const std::string& s );

    const std::string& destination() const;

    void set_destination( const std::string& s );

    /**
     * This method is needed to be able to create a duplicate of a child
     * capable of parsing their specific template type message.
     */
    //      virtual std::shared_ptr<signal_base> clone() = 0;

    /** Returns a DBus XML description of this interface */
    virtual std::string introspect( int = 0 ) const { return std::string(); }

    virtual std::string arg_name( size_t ) { return std::string(); }

    virtual void set_arg_name( size_t, const std::string& ) { }

protected:
    bool handle_dbus_outgoing( std::shared_ptr<const Message> );

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;
};

}

#endif
