// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "objectproxy.h"
#include <utility>
#include "callmessage.h"
#include "connection.h"
#include "interfaceproxy.h"
#include "standard-interfaces/peerinterfaceproxy.h"
#include "standard-interfaces/introspectableinterfaceproxy.h"
#include "standard-interfaces/propertiesinterfaceproxy.h"
#include "standard-interfaces/objectmanagerproxy.h"

namespace DBus {
class MethodProxyBase;
class PendingCall;
class ReturnMessage;

class ObjectProxy::priv_data {
public:
    priv_data( std::shared_ptr<Connection> conn, const std::string& destination, const std::string& path ) :
        m_connection( conn ),
        m_destination( destination ),
        m_path( path ),
        m_peerInterface( PeerInterfaceProxy::create() ),
        m_introspectableInterface( IntrospectableInterfaceProxy::create() ),
        m_propertiesInterface( PropertiesInterfaceProxy::create() ),
        m_objectManagerInterface( ObjectManagerProxy::create() )
    {}

    std::weak_ptr<Connection> m_connection;
    std::string m_destination;
    Path m_path;
    mutable std::shared_mutex m_interfaces_rwlock;
    std::mutex m_name_mutex;
    Interfaces m_interfaces;
    sigc::signal<void( std::shared_ptr<InterfaceProxy> )> m_signal_interface_added;
    sigc::signal<void( std::shared_ptr<InterfaceProxy> )> m_signal_interface_removed;
    std::shared_ptr<PeerInterfaceProxy> m_peerInterface;
    std::shared_ptr<IntrospectableInterfaceProxy> m_introspectableInterface;
    std::shared_ptr<PropertiesInterfaceProxy> m_propertiesInterface;
    std::shared_ptr<ObjectManagerProxy> m_objectManagerInterface;
};

ObjectProxy::ObjectProxy( std::shared_ptr<Connection> conn, const std::string& destination, const std::string& path ):
    m_priv( std::make_unique<priv_data>( conn, destination, path ) ) {
    add_interface( m_priv->m_peerInterface );
    add_interface( m_priv->m_introspectableInterface );
    add_interface( m_priv->m_propertiesInterface );
    add_interface( m_priv->m_objectManagerInterface );
}

std::shared_ptr<ObjectProxy> ObjectProxy::create( const std::string& path ) {
    return std::shared_ptr<ObjectProxy>( new ObjectProxy( std::shared_ptr<Connection>(), "", path ) );
}

std::shared_ptr<ObjectProxy> ObjectProxy::create( const std::string& destination, const std::string& path ) {
    return std::shared_ptr<ObjectProxy>( new ObjectProxy( std::shared_ptr<Connection>(), destination, path ) );
}

std::shared_ptr<ObjectProxy> ObjectProxy::create( std::shared_ptr<Connection> conn, const std::string& path ) {
    return std::shared_ptr<ObjectProxy>( new ObjectProxy( conn, "", path ) );
}

std::shared_ptr<ObjectProxy> ObjectProxy::create( std::shared_ptr<Connection> conn, const std::string& destination, const std::string& path ) {
    return std::shared_ptr<ObjectProxy>( new ObjectProxy( conn, destination, path ) );
}

ObjectProxy::~ObjectProxy() {
    auto connection =
        m_priv->m_connection.lock();

    if (connection)
        connection->unregister_object_proxy(this);

    remove_interface(m_priv->m_peerInterface);
    remove_interface(m_priv->m_introspectableInterface);
    remove_interface(m_priv->m_propertiesInterface);
}

std::weak_ptr<Connection> ObjectProxy::connection() const {
    return m_priv->m_connection;
}

void ObjectProxy::set_connection( std::shared_ptr<Connection> conn ) {
    m_priv->m_connection = conn;
}

const std::string& ObjectProxy::destination() const {
    return m_priv->m_destination;
}

void ObjectProxy::set_destination( const std::string& destination ) {
    m_priv->m_destination = destination;
}

const Path& ObjectProxy::path() const {
    return m_priv->m_path;
}

void ObjectProxy::set_path( const std::string& path ) {
    m_priv->m_path = path;

    for( Interfaces::iterator i = m_priv->m_interfaces.begin(); i != m_priv->m_interfaces.end(); i++ ) {
        i->second->on_object_set_path( path );
    }
}

const ObjectProxy::Interfaces& ObjectProxy::interfaces() const {
    return m_priv->m_interfaces;
}

std::shared_ptr<InterfaceProxy> ObjectProxy::interface_by_name( const std::string& name ) const {
    Interfaces::const_iterator iter;

    {
        std::shared_lock lock( m_priv->m_interfaces_rwlock );

        iter = m_priv->m_interfaces.find( name );
    }

    if( iter == m_priv->m_interfaces.end() ) { return std::shared_ptr<InterfaceProxy>(); }

    return iter->second;
}

bool ObjectProxy::add_interface( std::shared_ptr<InterfaceProxy> interface_ptr ) {
    bool result = true;

    if( !interface_ptr ) { return false; }

    if( interface_ptr->object() ) { interface_ptr->object()->remove_interface( interface_ptr ); }

    {
        std::unique_lock lock( m_priv->m_interfaces_rwlock );
        m_priv->m_interfaces.insert( std::make_pair( interface_ptr->name(), interface_ptr ) );

        interface_ptr->set_object( this );

    }

    m_priv->m_signal_interface_added.emit( interface_ptr );

    return result;
}

std::shared_ptr<InterfaceProxy> ObjectProxy::create_interface( const std::string& name ) {
    std::shared_ptr<InterfaceProxy> interface_ptr;

    interface_ptr = InterfaceProxy::create( name );

    if( this->add_interface( interface_ptr ) ) { return interface_ptr; }

    return std::shared_ptr<InterfaceProxy>();
}

void ObjectProxy::remove_interface( const std::string& name ) {
    Interfaces::iterator iter;
    std::shared_ptr<InterfaceProxy> interface_ptr, old_default;


    {
        std::unique_lock lock( m_priv->m_interfaces_rwlock );

        iter = m_priv->m_interfaces.find( name );

        if( iter != m_priv->m_interfaces.end() ) {
            interface_ptr = iter->second;
            m_priv->m_interfaces.erase( iter );
        }

    }

    if( interface_ptr ) { m_priv->m_signal_interface_removed.emit( interface_ptr ); }
}

void ObjectProxy::remove_interface( std::shared_ptr<InterfaceProxy> interface_ptr ) {
    Interfaces::iterator location;
    std::shared_ptr<InterfaceProxy> old_default;

    bool interface_removed = false;

    if( !interface_ptr ) { return; }

    {
        std::unique_lock lock( m_priv->m_interfaces_rwlock );

        location = m_priv->m_interfaces.find( interface_ptr->name() );

        if( location != m_priv->m_interfaces.end() ) {
            m_priv->m_interfaces.erase( location );
            interface_removed = true;
        }

    }

    if( interface_removed ) { m_priv->m_signal_interface_removed.emit( interface_ptr ); }
}

bool ObjectProxy::has_interface( const std::string& name ) const {
    Interfaces::const_iterator i;
    std::shared_lock lock( m_priv->m_interfaces_rwlock );

    i = m_priv->m_interfaces.find( name );

    return ( i != m_priv->m_interfaces.end() );
}

bool ObjectProxy::has_interface( std::shared_ptr<InterfaceProxy> interface_ptr ) const {
    if( !interface_ptr ) { return false; }

    Interfaces::const_iterator current, upper;
    bool result = false;
    std::shared_lock lock( m_priv->m_interfaces_rwlock );

    current = m_priv->m_interfaces.lower_bound( interface_ptr->name() );

    if( current != m_priv->m_interfaces.end() ) {
        upper = m_priv->m_interfaces.upper_bound( interface_ptr->name() );

        for( ; current != upper; current++ ) {
            if( current->second == interface_ptr ) {
                result = true;
                break;
            }
        }
    }

    return result;
}

bool ObjectProxy::add_method( const std::string& ifacename, std::shared_ptr<MethodProxyBase> method ) {
    if( !method ) { return false; }

    std::shared_ptr<InterfaceProxy> iface = this->interface_by_name( ifacename );

    if( !iface ) { iface = this->create_interface( ifacename ); }

    return iface->add_method( method );
}

std::shared_ptr<CallMessage> ObjectProxy::create_call_message( const std::string& interface_name, const std::string& method_name ) const {
    std::shared_ptr<CallMessage> call_message;

    if( m_priv->m_destination.empty() ) {
        call_message = CallMessage::create( m_priv->m_path, interface_name, method_name );
    } else {
        call_message = CallMessage::create( m_priv->m_destination, m_priv->m_path, interface_name, method_name );
    }

    return call_message;
}

std::shared_ptr<CallMessage> ObjectProxy::create_call_message( const std::string& method_name ) const {
    std::shared_ptr<CallMessage> call_message;

    if( m_priv->m_destination.empty() ) {
        call_message = CallMessage::create( m_priv->m_path, method_name );
    } else {
        call_message = CallMessage::create( m_priv->m_destination, m_priv->m_path, "", method_name );
    }

    return call_message;
}

std::shared_ptr<const ReturnMessage> ObjectProxy::call( std::shared_ptr<const CallMessage> call_message, int timeout_milliseconds ) const {
    std::shared_ptr<Connection> conn = m_priv->m_connection.lock();

    if( !conn ) { return std::shared_ptr<ReturnMessage>(); }

    return conn->send_with_reply_blocking( call_message, timeout_milliseconds );
}

std::shared_ptr<const ReturnMessage> ObjectProxy::call_notimeout( std::shared_ptr<const CallMessage> call_message ) const {
    std::shared_ptr<Connection> conn = m_priv->m_connection.lock();

    if( !conn ) { return std::shared_ptr<ReturnMessage>(); }

    return conn->send_with_reply_blocking_notimeout( call_message );
}

sigc::signal< void( std::shared_ptr<InterfaceProxy> )> ObjectProxy::signal_interface_added() {
    return m_priv->m_signal_interface_added;
}

sigc::signal< void( std::shared_ptr<InterfaceProxy> )> ObjectProxy::signal_interface_removed() {
    return m_priv->m_signal_interface_removed;
}

std::shared_ptr<PeerInterfaceProxy> ObjectProxy::getPeerInterface(){
    return m_priv->m_peerInterface;
}

std::shared_ptr<IntrospectableInterfaceProxy> ObjectProxy::getIntrospectableInterface(){
    return m_priv->m_introspectableInterface;
}

std::shared_ptr<PropertiesInterfaceProxy> ObjectProxy::getPropertiesInterface(){
    return m_priv->m_propertiesInterface;
}

std::shared_ptr<ObjectManagerProxy> ObjectProxy::getObjectManagerInterface(){
    return m_priv->m_objectManagerInterface;
}

}

