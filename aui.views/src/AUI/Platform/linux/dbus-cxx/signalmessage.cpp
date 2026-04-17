// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009 by Rick L. Vinyard, Jr.                            *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "signalmessage.h"
#include "error.h"
#include "message.h"
#include "validator.h"

namespace DBus {

SignalMessage::SignalMessage( ):
    Message() {
}

SignalMessage::SignalMessage( const std::string& name ):
    Message( ) {
    this->set_member( name );
}

SignalMessage::SignalMessage( const std::string& path, const std::string& interface_name, const std::string& name ) :
    Message() {
    set_path( path );
    set_interface( interface_name );
    set_member( name );
}

std::shared_ptr<SignalMessage> SignalMessage::create( ) {
    return std::shared_ptr<SignalMessage>( new SignalMessage() );
}

std::shared_ptr<SignalMessage> SignalMessage::create( const std::string& name ) {
    return std::shared_ptr<SignalMessage>( new SignalMessage( name ) );
}

std::shared_ptr<SignalMessage> SignalMessage::create( const std::string& path, const std::string& interface_name, const std::string& name ) {
    return std::shared_ptr<SignalMessage>( new SignalMessage( path, interface_name, name ) );
}

bool SignalMessage::set_path( const std::string& p ) {
    set_header_field( MessageHeaderFields::Path, Variant( Path( p ) ) );
    return true;
}

Path SignalMessage::path() const {
    DBus::Variant memberName = header_field( MessageHeaderFields::Path );

    if( memberName.type() == DataType::OBJECT_PATH ) {
        return memberName.to_path();
    }

    return std::string( "" );
}

//  bool SignalMessage::has_path( const std::string& p ) const
//  {
//    //return dbus_message_has_path( m_cobj, p.c_str() );
//  }

std::vector<std::string> SignalMessage::path_decomposed() const {
    std::vector<std::string> decomposed;
    //    char** p;
    //    dbus_message_get_path_decomposed( m_cobj, &p );
    //    for ( char** q=p; q != NULL; q++ )
    //      decomposed.push_back( *q );
    //    dbus_free_string_array( p );
    return decomposed;
}

bool SignalMessage::set_interface( const std::string& i ) {
    if( !Validator::validate_interface_name( i ) ) { return false; }

    set_header_field( MessageHeaderFields::Interface, Variant( i ) );
    return true;
}

std::string SignalMessage::interface_name() const {
    DBus::Variant memberName = header_field( MessageHeaderFields::Interface );

    if( memberName.type() == DataType::STRING ) {
        return memberName.to_string();
    }

    return std::string( "" );
}

bool SignalMessage::set_member( const std::string& m ) {
    set_header_field( MessageHeaderFields::Member, Variant( m ) );
    return true;
}

std::string SignalMessage::member() const {
    DBus::Variant memberName = header_field( MessageHeaderFields::Member );

    if( memberName.type() == DataType::STRING ) {
        return memberName.to_string();
    }

    return std::string( "" );
}


MessageType SignalMessage::type() const {
    return MessageType::SIGNAL;
}

}

