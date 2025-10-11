// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/message.h>
#include <memory>
#include <string>
#include <vector>
#include "enums.h"
#include "path.h"

#ifndef DBUSCXX_SIGNALMESSAGE_H
#define DBUSCXX_SIGNALMESSAGE_H

namespace DBus {

/**
 * Represents a DBus signal message
 *
 * This class inherits from Message and uses the dbus_message_* methods to
 * create an interface for a signal message that is received from a signal
 * or transmitted on signal emission.
 *
 * @ingroup message
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class SignalMessage : public Message {
protected:

    SignalMessage( );

    SignalMessage( const std::string& name );

    SignalMessage( const std::string& path, const std::string& interface_name, const std::string& name );

public:
    static std::shared_ptr<SignalMessage> create( );

    static std::shared_ptr<SignalMessage> create( const std::string& name );

    static std::shared_ptr<SignalMessage> create( const std::string& path, const std::string& interface_name, const std::string& name );

    bool set_path( const std::string& p );

    Path path() const;

    //      bool has_path( const std::string& p ) const;

    std::vector<std::string> path_decomposed() const;

    bool set_interface( const std::string& i );

    std::string interface_name() const;

    //bool has_interface( const std::string& i ) const;

    bool set_member( const std::string& m );

    std::string member() const;

    //bool has_member( const std::string& m ) const;

    virtual MessageType type() const;

};

}

#endif
