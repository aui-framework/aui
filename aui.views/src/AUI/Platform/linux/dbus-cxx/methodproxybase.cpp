// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009 by Rick L. Vinyard, Jr.                            *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "methodproxybase.h"
#include "callmessage.h"
#include "interfaceproxy.h"

#include <limits>

namespace DBus {

class PendingCall;
class ReturnMessage;

class MethodProxyBase::priv_data {
public:
    priv_data( const std::string& name ) :
        m_interface( nullptr ),
        m_name( name ),
        m_override_timeout( -1 ) {}

    InterfaceProxy* m_interface;
    const std::string m_name;
    // Override the timeout of call() member function. This is used when
    // interactive authorization is enabled.
    // Possible values:
    // -1: Interactive authorization is off, this variable should be
    //     ignored and the timeout_milliseconds argument of call() is
    //     respected.
    //  0: Interactive authorization is on, unlimited timeout is set.
    //     The timeout_milliseconds argument of call() is ignored.
    // >0: Interactive authorization is on, timeout equal to
    //     override_timeout is set. This timeout overrides the
    //     timeout_milliseconds argument of call().
    int m_override_timeout;
};


MethodProxyBase::MethodProxyBase( const std::string& name ):
    m_priv( std::make_unique<priv_data>( name ) ) {
}

MethodProxyBase::MethodProxyBase( const MethodProxyBase& other ) :
    m_priv( std::make_unique<priv_data>( other.m_priv->m_name ) ) {
    m_priv->m_interface = other.m_priv->m_interface;
}

std::shared_ptr<MethodProxyBase> MethodProxyBase::create( const std::string& name ) {
    return std::shared_ptr<MethodProxyBase>( new MethodProxyBase( name ) );
}

MethodProxyBase::~MethodProxyBase() {
}

InterfaceProxy* MethodProxyBase::interface_name() const {
    return m_priv->m_interface;
}

const std::string& MethodProxyBase::name() const {
    return m_priv->m_name;
}

std::shared_ptr<CallMessage> DBus::MethodProxyBase::create_call_message() const {
    if( !m_priv->m_interface ) { return std::shared_ptr<CallMessage>(); }

    std::shared_ptr<CallMessage> cm = m_priv->m_interface->create_call_message( m_priv->m_name );
    cm->set_no_reply( false );
    cm->set_interactive_authentication( m_priv->m_override_timeout != -1 );
    return cm;
}

std::shared_ptr<const ReturnMessage> DBus::MethodProxyBase::call( std::shared_ptr<const CallMessage> call_message, int timeout_milliseconds ) const {
    if( !m_priv->m_interface ) { return std::shared_ptr<const ReturnMessage>(); }

    int timeout = m_priv->m_override_timeout;
    switch ( timeout ) {
    case -1:
        return m_priv->m_interface->call( call_message, timeout_milliseconds );
    case 0:
        return m_priv->m_interface->call_notimeout( call_message );
    default:
        return m_priv->m_interface->call( call_message, timeout );
    }
}

void DBus::MethodProxyBase::enable_interactive_authorization( unsigned int timeout_milliseconds ) {
    constexpr auto int_max = std::numeric_limits<int>::max();

    if ( timeout_milliseconds > int_max ) {
        throw std::invalid_argument(
            "DBus::MethodProxyBase::enable_interactive_authorization() received "
            "too large timeout! Expected <=" + std::to_string( int_max ) + ", got " +
            std::to_string( timeout_milliseconds ) + "!"
        );
    }
    m_priv->m_override_timeout = timeout_milliseconds;
}

void DBus::MethodProxyBase::disable_interactive_authorization() {
    m_priv->m_override_timeout = -1;
}

//  std::shared_ptr<PendingCall> DBus::MethodProxyBase::call_async(std::shared_ptr<const CallMessage> call_message, int timeout_milliseconds) const
//  {
//    if ( not m_priv->m_interface ) return std::shared_ptr<PendingCall>();
//    return m_priv->m_interface->call_async(call_message, timeout_milliseconds);
//  }

void MethodProxyBase::set_interface( InterfaceProxy* proxy ) {
    m_priv->m_interface = proxy;
}

}

