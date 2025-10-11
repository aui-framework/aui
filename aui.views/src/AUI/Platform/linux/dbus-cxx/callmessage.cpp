// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/dbus-cxx-private.h>
#include "callmessage.h"
#include "enums.h"
#include "error.h"
#include "message.h"
#include "path.h"
#include "variant.h"
#include "returnmessage.h"
#include "errormessage.h"

namespace DBus {

CallMessage::CallMessage() : Message() {

}

CallMessage::CallMessage( const std::string& dest, const std::string& path, const std::string& iface, const std::string& method ) :
    Message() {
    std::ostringstream debug_msg;
    debug_msg << "Creating call message to " << dest << " path: " << path << " " << iface << "." << method;
    SIMPLELOGGER_DEBUG( "DBus.CallMessage", debug_msg.str() );
    set_path( path );
    set_interface( iface );
    set_member( method );
    set_destination( dest );
}

CallMessage::CallMessage( const std::string& path, const std::string& iface, const std::string& method ) :
    Message() {
    set_path( path );
    set_interface( iface );
    set_member( method );
}

CallMessage::CallMessage( const std::string& path, const std::string& method ) {
    set_path( path );
    set_member( method );
}

std::shared_ptr<CallMessage> CallMessage::create() {
    return std::shared_ptr<CallMessage>( new CallMessage() );
}

std::shared_ptr<CallMessage> CallMessage::create( const std::string& dest, const std::string& path, const std::string& iface, const std::string& method ) {
    return std::shared_ptr<CallMessage>( new CallMessage( dest, path, iface, method ) );
}

std::shared_ptr<CallMessage> CallMessage::create( const std::string& path, const std::string& iface, const std::string& method ) {
    return std::shared_ptr<CallMessage>( new CallMessage( path, iface, method ) );
}

std::shared_ptr<CallMessage> CallMessage::create( const std::string& path, const std::string& method ) {
    return std::shared_ptr<CallMessage>( new CallMessage( path, method ) );
}

std::shared_ptr<ReturnMessage> CallMessage::create_reply() const {
    if( !this->is_valid() ) { return std::shared_ptr<ReturnMessage>(); }

    std::shared_ptr<ReturnMessage> retmsg = ReturnMessage::create();
    retmsg->set_reply_serial( serial() );
    retmsg->set_destination( sender() );

    if( flags() & DBUSCXX_MESSAGE_NO_REPLY_EXPECTED ) {
        retmsg->invalidate();
    }

    return retmsg;
}

std::shared_ptr<ErrorMessage> CallMessage::create_error_reply() const {
    if( !this->is_valid() ) { return std::shared_ptr<ErrorMessage>(); }

    std::shared_ptr<ErrorMessage> retmsg = ErrorMessage::create();
    retmsg->set_reply_serial( serial() );
    retmsg->set_destination( sender() );

    if( flags() & DBUSCXX_MESSAGE_NO_REPLY_EXPECTED ) {
        retmsg->invalidate();
    }

    return retmsg;
}

void CallMessage::set_path( const std::string& p ) {
    set_header_field( MessageHeaderFields::Path, Variant( Path( p ) ) );
}

Path CallMessage::path() const {
    Variant field = header_field( MessageHeaderFields::Path );

    if( field.type() == DataType::OBJECT_PATH ) {
        return field.to_path();
    }

    return Path();
}

void CallMessage::set_interface( const std::string& i ) {
    set_header_field( MessageHeaderFields::Interface, Variant( i ) );
}

std::string CallMessage::interface_name() const {
    Variant iface = header_field( MessageHeaderFields::Interface );

    if( iface.type() == DataType::STRING ) {
        return iface.to_string();
    }

    return "";
}

void CallMessage::set_member( const std::string& m ) {
    set_header_field( MessageHeaderFields::Member, Variant( m ) );
}

std::string CallMessage::member() const {
    Variant member = header_field( MessageHeaderFields::Member );

    if( member.type() == DataType::STRING ) {
        return member.to_string();
    }

    return "";
}

void CallMessage::set_no_reply( bool no_reply ) {
    uint8_t newflags = flags();

    if( no_reply ) {
        newflags |= DBUSCXX_MESSAGE_NO_REPLY_EXPECTED;
    } else {
        newflags &= ( ~DBUSCXX_MESSAGE_NO_REPLY_EXPECTED );
    }

    set_flags( newflags );
}

void CallMessage::set_interactive_authentication( bool allow ) {
    uint8_t newflags = flags();

    if( allow ) {
        newflags |= DBUSCXX_MESSAGE_ALLOW_INTERACTIVE_AUTHORIZATION;
    } else {
        newflags &= ( ~DBUSCXX_MESSAGE_ALLOW_INTERACTIVE_AUTHORIZATION );
    }

    set_flags( newflags );
}

bool CallMessage::expects_reply() const {
    return flags() & DBUSCXX_MESSAGE_NO_REPLY_EXPECTED;
}

MessageType CallMessage::type() const {
    return MessageType::CALL;
}

}

