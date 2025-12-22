// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/message.h>
#include <AUI/Platform/linux/dbus-cxx/path.h>
#include <memory>
#include <string>
#include <vector>

#ifndef DBUSCXX_CALLMESSAGE_H
#define DBUSCXX_CALLMESSAGE_H

namespace DBus {

class ReplyMessage;
class ErrorMessage;

/**
 * Represents a DBus call message
 *
 * When this message is received, a matching exported method is called if found.
 * Depending on the result, either the response or an error must be sent back to the sender.
 * The reply should be created with either create_reply() or create_error_reply() depending
 * on the type of error that should be returned.
 *
 * @ingroup message
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class CallMessage : public Message {
protected:

    CallMessage();

    CallMessage( const std::string& dest, const std::string& path, const std::string& iface, const std::string& method );

    CallMessage( const std::string& path, const std::string& iface, const std::string& method );

    CallMessage( const std::string& path, const std::string& method );

public:

    static std::shared_ptr<CallMessage> create();

    static std::shared_ptr<CallMessage> create( const std::string& dest, const std::string& path, const std::string& iface, const std::string& method );

    static std::shared_ptr<CallMessage> create( const std::string& path, const std::string& iface, const std::string& method );

    static std::shared_ptr<CallMessage> create( const std::string& path, const std::string& method );

    /**
     * Create a reply to this call message.
     *
     * If no reply is expected, this will return a valid pointer, but an invalid
     * ReturnMessage so that the reply can be built but will be dropped before it
     * gets sent out.
     *
     * @return
     */
    std::shared_ptr<ReturnMessage> create_reply() const;

    /**
     * Create an error reply to this message.
     *
     * If no reply is expected, this will return a valid pointer, but an invalid
     * ErrorMessage so that the reply can be built but will be dropped before it
     * gets sent out.
     *
     * @return
     */
    std::shared_ptr<ErrorMessage> create_error_reply() const;

    void set_path( const std::string& p );

    Path path() const;

    void set_interface( const std::string& i );

    std::string interface_name() const;

    void set_member( const std::string& m );

    std::string member() const;

    void set_no_reply( bool no_reply = true );

    void set_interactive_authentication( bool allow );

    bool expects_reply() const;

    virtual MessageType type() const;

};

}

#endif
