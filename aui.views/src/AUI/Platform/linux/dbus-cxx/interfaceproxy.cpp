// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "interfaceproxy.h"
#include <map>
#include <utility>
#include "connection.h"
#include "methodproxybase.h"
#include "objectproxy.h"
#include "signalproxy.h"
#include <AUI/Platform/linux/dbus-cxx/dbus-cxx-private.h>

static const char* LOGGER_NAME = "DBus.InterfaceProxy";

namespace DBus {

class CallMessage;
class PendingCall;
class ReturnMessage;

class InterfaceProxy::priv_data {
public:
    priv_data( const std::string& name ):
        m_object( nullptr ),
        m_name( name ) {}

    ObjectProxy* m_object;
    const std::string m_name;
    Methods m_methods;
    Signals m_signals;
    mutable std::shared_mutex m_methods_rwlock;
    mutable std::shared_mutex m_properties_rwlock;
    std::map<std::string,std::shared_ptr<PropertyProxyBase>> m_properties;
    std::shared_ptr<DBus::SignalProxy<void(std::string,std::map<std::string,DBus::Variant>,std::vector<std::string>)>> m_updated_proxy;
};

InterfaceProxy::InterfaceProxy( const std::string& name ) {
    m_priv = std::make_unique<priv_data>( name );
}

std::shared_ptr<InterfaceProxy> InterfaceProxy::create( const std::string& name ) {
    return std::shared_ptr<InterfaceProxy>( new InterfaceProxy( name ) );
}

InterfaceProxy::~ InterfaceProxy( ) {
    if( m_priv->m_object ){
        std::shared_ptr<Connection> conn = m_priv->m_object->connection().lock();
        if( !conn ){
            return;
        }

        conn->remove_free_signal_proxy( m_priv->m_updated_proxy );
    }
}

ObjectProxy* InterfaceProxy::object() const {
    return m_priv->m_object;
}

void InterfaceProxy::set_object( ObjectProxy* obj ) {
    m_priv->m_object = obj;

    std::shared_ptr<Connection> conn = connection().lock();
    if( conn ){
        m_priv->m_updated_proxy =
            conn->create_free_signal_proxy<void(std::string,std::map<std::string,DBus::Variant>,std::vector<std::string>)>(
                DBus::MatchRuleBuilder::create()
                .set_path( path() )
                .set_interface( DBUS_CXX_PROPERTIES_INTERFACE )
                .set_member( "PropertiesChanged" )
                .as_signal_match()
                );

        m_priv->m_updated_proxy->connect( sigc::mem_fun( *this, &InterfaceProxy::property_updated ) );

        for( std::shared_ptr<SignalProxyBase> sig : m_priv->m_signals ){
            // If we have created SignalProxies before the set_object call
            // (by, for example, creating them in the constructor)
            // the path will have not been set.  To fix this, remove any matches
            // that we may have added already, set the new path to use, and update
            // the match rule on the signal so we can re-add it.
            conn->remove_match( sig->match_rule() );
            sig->set_path( path() );
            sig->update_match_rule();
            conn->add_match( sig->match_rule() );
        }
    }
}

Path InterfaceProxy::path() const {
    if( m_priv->m_object ) { return m_priv->m_object->path(); }

    return Path();
}

std::weak_ptr<Connection> InterfaceProxy::connection() const {
    if( m_priv->m_object ) { return m_priv->m_object->connection(); }

    return std::weak_ptr<Connection>();
}

const std::string& InterfaceProxy::name() const {
    return m_priv->m_name;
}

const InterfaceProxy::Methods& InterfaceProxy::methods() const {
    return m_priv->m_methods;
}

std::shared_ptr<MethodProxyBase> InterfaceProxy::method( const std::string& name ) const {
    Methods::const_iterator iter;
    std::shared_lock lock( m_priv->m_methods_rwlock );

    iter = m_priv->m_methods.find( name );

    if( iter == m_priv->m_methods.end() ) { return std::shared_ptr<MethodProxyBase>(); }

    return iter->second;
}

bool InterfaceProxy::add_method( std::shared_ptr<MethodProxyBase> method ) {
    bool result = true;

    if( !method ) { return false; }

    if( this->has_method( method ) ) { return false; }

    if( method->interface_name() ) { method->interface_name()->remove_method( method ); }

    {
        std::unique_lock lock( m_priv->m_methods_rwlock );

        m_priv->m_methods.insert( std::make_pair( method->name(), method ) );
        method->set_interface( this );
    }

    return result;
}

void InterfaceProxy::remove_method( const std::string& name ) {
    Methods::iterator iter;
    std::shared_ptr<MethodProxyBase> method;

    {
        std::unique_lock lock( m_priv->m_methods_rwlock );

        iter = m_priv->m_methods.find( name );

        if( iter != m_priv->m_methods.end() ) {
            method = iter->second;
            m_priv->m_methods.erase( iter );
        }
    }
    method->set_interface( nullptr );
}

void InterfaceProxy::remove_method( std::shared_ptr<MethodProxyBase> method ) {
    Methods::iterator location;
    bool erased = false;

    if( !method ) { return; }

    {
        std::unique_lock lock( m_priv->m_methods_rwlock );

        location = m_priv->m_methods.find( method->name() );

        if( location != m_priv->m_methods.end() ) {
            m_priv->m_methods.erase( location );
            erased = true;
        }
    }

    method->set_interface( nullptr );
}

bool InterfaceProxy::has_method( const std::string& name ) const {
    Methods::const_iterator iter;
    std::shared_lock lock( m_priv->m_methods_rwlock );

    iter = m_priv->m_methods.find( name );

    return ( iter != m_priv->m_methods.end() );
}

bool InterfaceProxy::has_method( std::shared_ptr<MethodProxyBase> method ) const {
    Methods::const_iterator current, upper;
    bool found = false;

    if( !method ) { return false; }

    std::shared_lock lock( m_priv->m_methods_rwlock );

    current = m_priv->m_methods.lower_bound( method->name() );

    if( current != m_priv->m_methods.end() ) {
        upper = m_priv->m_methods.upper_bound( method->name() );

        for( ; current != upper; current++ ) {
            if( current->second == method ) {
                found = true;
                break;
            }
        }
    }

    return found;
}

std::shared_ptr<CallMessage> InterfaceProxy::create_call_message( const std::string& method_name ) const {
    if( !m_priv->m_object ) { return std::shared_ptr<CallMessage>(); }

    return m_priv->m_object->create_call_message( m_priv->m_name, method_name );
}

std::shared_ptr<const ReturnMessage> InterfaceProxy::call( std::shared_ptr<const CallMessage> call_message, int timeout_milliseconds ) const {
    if( !m_priv->m_object ) { return std::shared_ptr<const ReturnMessage>(); }

    return m_priv->m_object->call( call_message, timeout_milliseconds );
}

std::shared_ptr<const ReturnMessage> InterfaceProxy::call_notimeout( std::shared_ptr<const CallMessage> call_message ) const {
    if( !m_priv->m_object ) { return std::shared_ptr<const ReturnMessage>(); }

    return m_priv->m_object->call_notimeout( call_message );
}

//  std::shared_ptr<PendingCall> InterfaceProxy::call_async( std::shared_ptr<const CallMessage> call_message, int timeout_milliseconds ) const
//  {
//    if ( not m_priv->m_object ) return std::shared_ptr<PendingCall>();
//    return m_priv->m_object->call_async( call_message, timeout_milliseconds );
//  }

const InterfaceProxy::Signals& InterfaceProxy::signals() const {
    return m_priv->m_signals;
}

std::shared_ptr<SignalProxyBase> InterfaceProxy::signal( const std::string& signame ) {
    for( Signals::iterator i = m_priv->m_signals.begin(); i != m_priv->m_signals.end(); i++ ) {
        if( ( *i )->name() == signame ) { return *i; }
    }

    return std::shared_ptr<SignalProxyBase>();
}

bool InterfaceProxy::add_signal( std::shared_ptr<SignalProxyBase> sig ) {
    // is it a valid signal?
    if( !sig ) { return false; }

    // is it already added?
    if( m_priv->m_signals.find( sig ) != m_priv->m_signals.end() ) { return false; }

    // make sure the path and interface are set
    sig->set_path( this->path() );
    sig->set_interface( this->name() );

    // add it to the signal set
    m_priv->m_signals.insert( sig );

    std::shared_ptr<Connection> conn = connection().lock();
    if( conn ){
        conn->add_match( sig->match_rule() );
    }

    return true;
}

bool InterfaceProxy::remove_signal( const std::string& signame ) {
    return this->remove_signal( this->signal( signame ) );
}

bool InterfaceProxy::remove_signal( std::shared_ptr<SignalProxyBase> sig ) {
    if( !sig ) { return false; }

    if( !this->has_signal( sig ) ) { return false; }

    m_priv->m_signals.erase( sig );
    return true;
}

bool InterfaceProxy::has_signal( const std::string& signame ) const {
    for( Signals::iterator i = m_priv->m_signals.begin(); i != m_priv->m_signals.end(); i++ ) {
        if( ( *i )->name() == signame ) { return true; }
    }

    return false;
}

bool InterfaceProxy::has_signal( std::shared_ptr<SignalProxyBase> sig ) const {
    return m_priv->m_signals.find( sig ) != m_priv->m_signals.end();
}

void InterfaceProxy::on_object_set_path( const std::string& path ) {
    for( Signals::iterator i = m_priv->m_signals.begin(); i != m_priv->m_signals.end(); i++ ) {
        ( *i )->set_path( path );
    }
}

const std::map<std::string,std::shared_ptr<PropertyProxyBase>>& InterfaceProxy::properties() const {
    return m_priv->m_properties;
}

std::shared_ptr<PropertyProxyBase> InterfaceProxy::property( const std::string& name ) const {
    std::map<std::string,std::shared_ptr<PropertyProxyBase>>::const_iterator iter;
    std::shared_lock lock( m_priv->m_properties_rwlock );

    iter = m_priv->m_properties.find( name );

    if( iter == m_priv->m_properties.end() ) { return std::shared_ptr<PropertyProxyBase>(); }

    return iter->second;
}

bool InterfaceProxy::add_property( std::shared_ptr<PropertyProxyBase> property ) {
    bool result = true;

    if( !property ) { return false; }

    if( this->has_property( property ) ) { return false; }

    if( property->interface_name() ) { property->interface_name()->remove_property( property ); }

    {
        std::unique_lock lock( m_priv->m_properties_rwlock );

        m_priv->m_properties.insert( std::make_pair( property->name(), property ) );
        property->set_interface( this );
    }

    return result;
}

bool InterfaceProxy::has_property( const std::string& name ) const {
    std::map<std::string,std::shared_ptr<PropertyProxyBase>>::const_iterator iter;
    std::shared_lock lock( m_priv->m_properties_rwlock );

    iter = m_priv->m_properties.find( name );

    return ( iter != m_priv->m_properties.end() );
}

bool InterfaceProxy::has_property( std::shared_ptr<PropertyProxyBase> property ) const {
    std::map<std::string,std::shared_ptr<PropertyProxyBase>>::const_iterator current, upper;
    bool found = false;

    if( !property ) { return false; }

    std::shared_lock lock( m_priv->m_properties_rwlock );

    current = m_priv->m_properties.lower_bound( property->name() );

    if( current != m_priv->m_properties.end() ) {
        upper = m_priv->m_properties.upper_bound( property->name() );

        for( ; current != upper; current++ ) {
            if( current->second == property ) {
                found = true;
                break;
            }
        }
    }

    return found;
}

void InterfaceProxy::remove_property( const std::string& name ) {
    std::map<std::string,std::shared_ptr<PropertyProxyBase>>::iterator iter;
    std::shared_ptr<PropertyProxyBase> property;

    {
        std::unique_lock lock( m_priv->m_properties_rwlock );

        iter = m_priv->m_properties.find( name );

        if( iter != m_priv->m_properties.end() ) {
            property = iter->second;
            m_priv->m_properties.erase( iter );
        }
    }
    property->set_interface( nullptr );
}

void InterfaceProxy::remove_property( std::shared_ptr<PropertyProxyBase> property ) {
    std::map<std::string,std::shared_ptr<PropertyProxyBase>>::iterator location;
    bool erased = false;

    if( !property ) { return; }

    {
        std::unique_lock lock( m_priv->m_methods_rwlock );

        location = m_priv->m_properties.find( property->name() );

        if( location != m_priv->m_properties.end() ) {
            m_priv->m_properties.erase( location );
            erased = true;
        }
    }

    property->set_interface( nullptr );
}

void InterfaceProxy::cache_properties(){
    std::shared_ptr<CallMessage> msg =
            CallMessage::create( m_priv->m_object->destination(), m_priv->m_object->path(), "org.freedesktop.DBus.Properties", "GetAll" );
    msg << m_priv->m_name;
    std::shared_ptr<const ReturnMessage> ret =
            call( msg );

    MessageIterator iter = ret->begin();
    std::map<std::string,DBus::Variant> allValues;

    iter >> allValues;

    for( std::map<std::string,DBus::Variant>::const_iterator it = allValues.cbegin();
         it != allValues.cend();
         it++ ){
        SIMPLELOGGER_TRACE( LOGGER_NAME, "Caching property " << it->first << "=" << it->second );
        std::shared_ptr<DBus::PropertyProxyBase> prop = property( it->first );
        prop->updated_value( it->second );
    }
}

void InterfaceProxy::property_updated( std::string iface,
                                  std::map<std::string,DBus::Variant> changed,
                                  std::vector<std::string> invalidated ){
    if( m_priv->m_name != iface ){
        // Wrong interface
        return;
    }

    std::shared_lock lock( m_priv->m_properties_rwlock );
    for( std::pair<std::string,DBus::Variant> entry : changed ){
        std::shared_ptr<PropertyProxyBase> prop = property( entry.first );

        if( prop ){
            SIMPLELOGGER_TRACE( LOGGER_NAME, "Updating property '"
                                << prop->name()
                                << "' on interface '"
                                << m_priv->m_name
                                << "' = " << entry.second);
            prop->updated_value( entry.second );
        }
    }
}

}

