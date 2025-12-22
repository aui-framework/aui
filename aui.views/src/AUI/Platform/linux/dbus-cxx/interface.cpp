// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "interface.h"
#include <AUI/Platform/linux/dbus-cxx/object.h>
#include <map>
#include <utility>
#include "callmessage.h"
#include "dbus-cxx-private.h"
#include "methodbase.h"
#include "signalbase.h"
#include "connection.h"

static const char* LOGGER_NAME = "DBus.Interface";

namespace DBus {
class Connection;

class Interface::priv_data {
public:
    priv_data( std::string name ):
        m_name( name ) {}

    const std::string m_name;
    std::string m_path;
    Methods m_methods;
    Signals m_signals;
    std::set<std::shared_ptr<PropertyBase>> m_properties;
    mutable std::shared_mutex m_methods_rwlock;
    mutable std::shared_mutex m_signals_rwlock;
    mutable std::shared_mutex m_properties_rwlock;
    sigc::signal<void( std::shared_ptr<MethodBase> )> m_signal_method_added;
    sigc::signal<void( std::shared_ptr<MethodBase> )> m_signal_method_removed;
    std::weak_ptr<DBus::Connection> m_connection;
};

Interface::Interface( const std::string& name ) {
    m_priv = std::make_unique<priv_data>( name );
}

std::shared_ptr<Interface> Interface::create( const std::string& name ) {
    return std::shared_ptr<Interface>( new Interface( name ) );
}

Interface::~ Interface( ) {
}


Path Interface::path() const {
    return Path( m_priv->m_path );
}

const std::string& Interface::name() const {
    return m_priv->m_name;
}

const Interface::Methods& Interface::methods() const {
    return m_priv->m_methods;
}

std::shared_ptr<MethodBase> Interface::method( const std::string& name ) const {
    Methods::const_iterator iter;

    {
        std::shared_lock lock( m_priv->m_methods_rwlock );

        iter = m_priv->m_methods.find( name );
    }

    if( iter == m_priv->m_methods.end() ) { return std::shared_ptr<MethodBase>(); }

    return iter->second;
}

bool Interface::add_method( std::shared_ptr<MethodBase> method ) {
    bool result = true;

    if( !method ) { return false; }

    if( has_method( method->name() ) ) { return false; }

    {
        std::unique_lock lock( m_priv->m_methods_rwlock );

        m_priv->m_methods.insert( std::make_pair( method->name(), method ) );
    }

    m_priv->m_signal_method_added.emit( method );

    return result;
}

bool Interface::remove_method( const std::string& name ) {
    Methods::iterator iter;
    std::shared_ptr<MethodBase> method;

    {
        std::unique_lock lock( m_priv->m_methods_rwlock );

        iter = m_priv->m_methods.find( name );

        if( iter != m_priv->m_methods.end() ) {
            method = iter->second;
            m_priv->m_methods.erase( iter );
        }
    }

    if( method ) {
        m_priv->m_signal_method_removed.emit( method );
        return true;
    }

    return false;
}

bool Interface::remove_method( std::shared_ptr<MethodBase> torem ) {
    Methods::iterator iter;
    std::shared_ptr<MethodBase> method;

    {
        std::unique_lock lock( m_priv->m_methods_rwlock );

        iter = m_priv->m_methods.find( torem->name() );

        if( iter != m_priv->m_methods.end() ) {
            method = iter->second;
        }

        if( method == torem ) {
            m_priv->m_methods.erase( iter );
        }
    }

    if( method ) {
        m_priv->m_signal_method_removed.emit( method );
        return true;
    }

    return false;
}

bool Interface::has_method( const std::string& name ) const {
    Methods::const_iterator iter;
    std::shared_lock lock( m_priv->m_methods_rwlock );

    iter = m_priv->m_methods.find( name );

    return ( iter != m_priv->m_methods.end() );
}

bool Interface::add_signal( std::shared_ptr<SignalBase> sig ) {
    bool result = false;

    if( !sig ) { return false; }

    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Interface(" << this->name() << ")::add_signal (" << sig->name() << ")" );

    std::unique_lock lock( m_priv->m_signals_rwlock );

    // Do we already have the signal?
    if( m_priv->m_signals.find( sig ) != m_priv->m_signals.end() ) {
        SIMPLELOGGER_DEBUG( LOGGER_NAME, "Interface(" << this->name() << ")::add_signal (" << sig->name() << ") failed since signal is already present" );
        // If so, we won't add it again
        result = false;
    } else {
        SIMPLELOGGER_DEBUG( LOGGER_NAME, "Interface(" << this->name() << ")::add_signal (" << sig->name() << ") succeeded" );
        m_priv->m_signals.insert( sig );
        sig->set_path( this->path() );
        sig->set_interface( m_priv->m_name );
        result = true;
    }

    sig->set_connection( m_priv->m_connection );

    return result;
}

bool Interface::remove_signal( std::shared_ptr<SignalBase> signal ) {
    bool result = false;
    std::unique_lock lock( m_priv->m_signals_rwlock );

    Signals::iterator i = m_priv->m_signals.find( signal );

    if( i != m_priv->m_signals.end() ) {
        m_priv->m_signals.erase( i );
        result = true;
    }

    return result;
}

bool Interface::remove_signal( const std::string& name ) {
    bool result = false;
    std::unique_lock lock( m_priv->m_signals_rwlock );

    Signals::iterator i = m_priv->m_signals.begin();

    while( i != m_priv->m_signals.end() ) {
        if( ( *i )->name() == name ) {
            Signals::iterator temp = i++;
            m_priv->m_signals.erase( temp );
        } else {
            i++;
        }
    }

    return result;
}

bool Interface::has_signal( std::shared_ptr<SignalBase> signal ) const {
    bool result;
    std::shared_lock lock( m_priv->m_signals_rwlock );

    result = m_priv->m_signals.find( signal ) != m_priv->m_signals.end();

    return result;
}

bool Interface::has_signal( const std::string& name ) const {
    bool result = false;
    std::shared_lock lock( m_priv->m_signals_rwlock );

    for( Signals::iterator i = m_priv->m_signals.begin(); i != m_priv->m_signals.end(); i++ ) {
        if( ( *i )->name() == name ) {
            result = true;
            break;
        }
    }

    return result;
}

const Interface::Signals& Interface::signals() {
    return m_priv->m_signals;
}

std::shared_ptr<SignalBase> Interface::signal( const std::string& signal_name ) {
    std::shared_ptr<SignalBase> sig;
    std::shared_lock lock( m_priv->m_signals_rwlock );

    for( Signals::iterator i = m_priv->m_signals.begin(); i != m_priv->m_signals.end(); i++ ) {
        if( ( *i )->name() == signal_name ) {
            sig = *i;
            break;
        }
    }

    return sig;
}

sigc::signal< void( std::shared_ptr<MethodBase> )> Interface::signal_method_added() {
    return m_priv->m_signal_method_added;
}

sigc::signal< void( std::shared_ptr<MethodBase> )> Interface::signal_method_removed() {
    return m_priv->m_signal_method_removed;
}

std::string Interface::introspect( int space_depth ) const {
    std::ostringstream sout;
    std::string spaces;
    Methods::const_iterator miter;
    Signals::const_iterator siter;
    std::set<std::shared_ptr<PropertyBase>>::const_iterator piter;

    for( int i = 0; i < space_depth; i++ ) { spaces += " "; }

    sout << spaces << "<interface name=\"" << this->name() << "\">\n";

    for( miter = m_priv->m_methods.begin(); miter != m_priv->m_methods.end(); miter++ ) {
        sout << miter->second->introspect( space_depth + 2 );
    }

    for( siter = m_priv->m_signals.begin(); siter != m_priv->m_signals.end(); siter++ ) {
        sout << ( *siter )->introspect( space_depth + 2 );
    }

    for( piter = m_priv->m_properties.begin(); piter != m_priv->m_properties.end(); piter++ ){
        sout << ( *piter )->introspect( space_depth + 2 );
    }

    sout << spaces << "</interface>\n";
    return sout.str();
}

HandlerResult Interface::handle_call_message( std::shared_ptr<Connection> conn, std::shared_ptr<const CallMessage> message ) {
    SIMPLELOGGER_DEBUG( LOGGER_NAME, "handle_call_message  interface=" << m_priv->m_name );

    Methods::iterator method_it;
    std::shared_ptr<MethodBase> method;

    method_it = m_priv->m_methods.find( message->member() );

    if( method_it == m_priv->m_methods.end() ) {
        return HandlerResult::Invalid_Method;
    }

    return method_it->second->handle_call_message( conn, message );
}

HandlerResult Interface::handle_properties_message( std::shared_ptr<Connection> conn, std::shared_ptr<const CallMessage> message ){
    SIMPLELOGGER_DEBUG( LOGGER_NAME, "handle_properties_message  interface=" << m_priv->m_name );
    std::shared_lock lock( m_priv->m_properties_rwlock );
    std::shared_ptr<ReturnMessage> retmsg = message->create_reply();
    std::string errMsg;
    std::string errName = DBUSCXX_ERROR_UNKNOWN_PROPERTY;

    if( message->member() == "GetAll" ){
        std::map<std::string,DBus::Variant> retval;

        for( std::shared_ptr<DBus::PropertyBase> prop : m_priv->m_properties ){
            if( prop->variant_value().type() == DataType::INVALID ){
                // The property has not been set, so we must assume that it does not exist
                continue;
            }

            switch( prop->access_type() ){
            case DBus::PropertyAccess::ReadOnly:
            case DBus::PropertyAccess::ReadWrite:
                retval[ prop->name() ] = prop->variant_value();
                break;
            case DBus::PropertyAccess::WriteOnly:
                continue;
            }
        }

        retmsg << retval;
        conn << retmsg;
        return HandlerResult::Handled;
    } else if( message->member() == "Get" ){
        std::string interfaceName;
        std::string propertyName;

        message >> interfaceName >> propertyName;

        for( std::shared_ptr<DBus::PropertyBase> prop : m_priv->m_properties ){
            if( prop->variant_value().type() == DataType::INVALID ){
                // The property has not been set, so we must assume that it does not exist
                continue;
            }

            if( prop->name() == propertyName ){
                if( prop->access_type() != DBus::PropertyAccess::WriteOnly ){
                    retmsg << prop->variant_value();
                    conn << retmsg;
                    return HandlerResult::Handled;
                }
            }
        }

        errMsg = "Unable to find property " + propertyName + " on interface " + interfaceName;
    } else if( message->member() == "Set" ){
        std::string interfaceName;
        std::string propertyName;
        DBus::Variant variantValue;

        message >> interfaceName >> propertyName >> variantValue;
        errMsg = "Unable to find property " + propertyName + " on interface " + interfaceName;

        for( std::shared_ptr<DBus::PropertyBase> prop : m_priv->m_properties ){
            if( prop->variant_value().type() == DataType::INVALID ){
                // The property has not been set, so we must assume that it does not exist
                continue;
            }

            if( prop->name() == propertyName ){
                if( prop->access_type() != DBus::PropertyAccess::ReadOnly ){
                    prop->set_value( variantValue );
                    conn << retmsg;
                    return HandlerResult::Handled;
                }
                errName = DBUSCXX_ERROR_PROPERTY_READ_ONLY;
                errMsg = "Property " + propertyName + " on interface " + interfaceName + " is read-only";
                break;
            }
        }
    }

    std::shared_ptr<ErrorMessage> errmsg = message->create_error_reply();
    errmsg->set_name( errName );
    errmsg->set_message( errMsg );
    conn << errmsg;
    return HandlerResult::Handled;
}

void Interface::set_path( const std::string& new_path ) {
    m_priv->m_path = new_path;

    for( Signals::iterator i = m_priv->m_signals.begin(); i != m_priv->m_signals.end(); i++ ) {
        ( *i )->set_path( new_path );
    }
}

bool Interface::add_property( std::shared_ptr<PropertyBase> prop ) {
    bool result = true;

    if( !prop ) { return false; }

    if( has_property( prop->name() ) ) { return false; }

    {
        std::unique_lock lock( m_priv->m_properties_rwlock );

        m_priv->m_properties.insert( prop );
    }

    //m_priv->m_signal_method_added.emit( method );
    prop->setInterface( this );

    return result;
}

bool Interface::has_property( const std::string& name ) const {
    bool result = false;
    std::shared_lock lock( m_priv->m_properties_rwlock );

    for( std::set<std::shared_ptr<PropertyBase>>::iterator i = m_priv->m_properties.begin();
         i != m_priv->m_properties.end();
         i++ ) {
        if( ( *i )->name() == name ) {
            result = true;
            break;
        }
    }

    return result;
}

void Interface::property_updated( DBus::PropertyBase* prop ){
    std::shared_ptr<SignalMessage> sigChanged = SignalMessage::create( m_priv->m_path,
                                                                       DBUS_CXX_PROPERTIES_INTERFACE,
                                                                       "PropertiesChanged" );

    std::map<std::string,DBus::Variant> changed;
    std::vector<std::string> invalidated;

    changed[ prop->name() ] = prop->variant_value();

    sigChanged << m_priv->m_name << changed << invalidated;

    std::shared_ptr<Connection> conn = m_priv->m_connection.lock();
    if( !conn ){
        return;
    }

    conn << sigChanged;
}

void Interface::set_connection( std::weak_ptr<Connection> conn ){
    m_priv->m_connection = conn;

    std::unique_lock lock( m_priv->m_signals_rwlock );
    for( std::shared_ptr<SignalBase> signal : m_priv->m_signals ){
        signal->set_connection( conn );
    }
}

const std::set<std::shared_ptr<PropertyBase>>& Interface::properties(){
    return m_priv->m_properties;
}

}

