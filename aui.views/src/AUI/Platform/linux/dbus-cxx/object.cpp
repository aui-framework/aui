// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "object.h"
#include <cstring>
#include <sstream>
#include <fstream>
#include <utility>
#include "callmessage.h"
#include "connection.h"
#include "dbus-cxx-private.h"
#include "interface.h"
#include "message.h"
#include "path.h"
#include "utility.h"

static const char* LOGGER_NAME = "DBus.Object";

namespace DBus {
class ErrorInvalidMessageType;
class ReturnMessage;

typedef std::map<std::shared_ptr<Interface>, sigc::connection> InterfaceSignalNameConnections;

class Object::priv_data {
public:
    priv_data() {}

    Children m_children;
    mutable std::shared_mutex m_interfaces_rwlock;
    std::mutex m_name_mutex;
    Interfaces m_interfaces;
    std::shared_ptr<Interface>  m_default_interface;
    sigc::signal<void( std::shared_ptr<Interface>, std::shared_ptr<Interface> ) > m_signal_default_interface_changed;
    sigc::signal<void( std::shared_ptr<Interface> ) > m_signal_interface_added;
    sigc::signal<void( std::shared_ptr<Interface> ) > m_signal_interface_removed;
    InterfaceSignalNameConnections m_interface_signal_name_connections;
    std::weak_ptr<Connection> m_connection;
    Path m_path;
    sigc::signal<void( std::shared_ptr<Connection> ) > m_signal_registered;
    sigc::signal<void( std::shared_ptr<Connection> ) > m_signal_unregistered;
    std::thread::id m_calling_thread;
    bool m_is_lightweight;
    bool m_has_object_manager;
    std::vector<sigc::connection> m_interface_added_connections;
    std::vector<sigc::connection> m_interface_removed_connections;
};

Object::Object( const std::string& path ):
    m_priv( std::make_unique<priv_data>() ) {
    m_priv->m_path = path;
    m_priv->m_is_lightweight = false;
    m_priv->m_has_object_manager = false;
}

std::shared_ptr<Object> Object::create( const std::string& path ) {
    return std::shared_ptr<Object>( new Object( path ) );
}

std::shared_ptr<Object> Object::create_lightweight( const std::string& path ){
    std::shared_ptr<Object> obj = std::shared_ptr<Object>( new Object( path ) );
    obj->m_priv->m_is_lightweight = true;
    return obj;
}

Object::~ Object( ) {
}

const Path& Object::path() const {
    return m_priv->m_path;
}

std::weak_ptr< Connection > Object::connection() const {
    return m_priv->m_connection;
}

bool Object::unregister() {
    std::shared_ptr connection = m_priv->m_connection.lock();

    if( !connection ) { return true; }

    return connection->unregister_object( m_priv->m_path );
}

sigc::signal< void( std::shared_ptr<Connection> )>& Object::signal_registered() {
    return m_priv->m_signal_registered;
}

sigc::signal< void( std::shared_ptr<Connection> )>& Object::signal_unregistered() {
    return m_priv->m_signal_unregistered;
}

void Object::set_connection( std::shared_ptr<Connection> conn ) {
    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Object::set_connection" );
    std::shared_ptr<Connection> current = m_priv->m_connection.lock();
    if( current && (current != conn) ){
        unregister();
    }
    m_priv->m_connection = conn;

    for( Children::iterator c = m_priv->m_children.begin(); c != m_priv->m_children.end(); c++ ) {
        c->second->set_connection( conn );
    }

    std::shared_lock lock( m_priv->m_interfaces_rwlock );
    for( std::pair<std::string,std::shared_ptr<Interface>> pair : m_priv->m_interfaces ){
        pair.second->set_connection( conn );
    }
}

const Object::Interfaces& Object::interfaces() const {
    return m_priv->m_interfaces;
}

std::shared_ptr<Interface> Object::interface_by_name( const std::string& name ) const {
    Interfaces::const_iterator iter;

    {
        std::shared_lock lock( m_priv->m_interfaces_rwlock );

        iter = m_priv->m_interfaces.find( name );
    }

    if( iter == m_priv->m_interfaces.end() ) { return std::shared_ptr<Interface>(); }

    return iter->second;
}

bool Object::add_interface( std::shared_ptr<Interface> interface_ptr ) {
    bool result = true;

    if( !interface_ptr ) { return false; }

    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Object::add_interface " << interface_ptr->name() );

    {
        std::unique_lock lock( m_priv->m_interfaces_rwlock );

        InterfaceSignalNameConnections::iterator i;

        i = m_priv->m_interface_signal_name_connections.find( interface_ptr );

        if( i == m_priv->m_interface_signal_name_connections.end() ) {
            m_priv->m_interfaces.insert( std::make_pair( interface_ptr->name(), interface_ptr ) );

            interface_ptr->set_path( path() );
            interface_ptr->set_connection( connection() );
        } else {
            result = false;
        }
    }

    m_priv->m_signal_interface_added.emit( interface_ptr );

    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Object::add_interface " << interface_ptr->name() << " successful: " << result );

    return result;
}

std::shared_ptr<Interface> Object::create_interface( const std::string& name ) {
    std::shared_ptr<Interface> interface_ptr;

    interface_ptr = Interface::create( name );

    if( !this->add_interface( interface_ptr ) ) { return std::shared_ptr<Interface>(); }

    if( !m_priv->m_default_interface && name.empty() ) { this->set_default_interface( interface_ptr->name() ); }

    return interface_ptr;
}

void Object::remove_interface( const std::string& name ) {
    Interfaces::iterator iter;
    std::shared_ptr<Interface> interface_ptr;
    InterfaceSignalNameConnections::iterator i;

    {
        std::unique_lock lock( m_priv->m_interfaces_rwlock );

        iter = m_priv->m_interfaces.find( name );

        if( iter != m_priv->m_interfaces.end() ) {
            interface_ptr = iter->second;
            m_priv->m_interfaces.erase( iter );
        }

        if( interface_ptr ) {
            i = m_priv->m_interface_signal_name_connections.find( interface_ptr );

            if( i != m_priv->m_interface_signal_name_connections.end() ) {
                i->second.disconnect();
                m_priv->m_interface_signal_name_connections.erase( i );
            }
        }
    }

    if( interface_ptr ) { m_priv->m_signal_interface_removed.emit( interface_ptr ); }
}

bool Object::has_interface( const std::string& name ) {
    Interfaces::const_iterator i;
    std::shared_lock lock( m_priv->m_interfaces_rwlock );

    i = m_priv->m_interfaces.find( name );

    return ( i != m_priv->m_interfaces.end() );
}

std::shared_ptr<Interface> Object::default_interface() const {
    return m_priv->m_default_interface;
}

bool Object::set_default_interface( const std::string& new_default_name ) {
    Interfaces::iterator iter;
    std::shared_ptr<Interface> old_default;
    bool result = false;

    {
        std::shared_lock lock( m_priv->m_interfaces_rwlock );

        iter = m_priv->m_interfaces.find( new_default_name );

        if( iter != m_priv->m_interfaces.end() ) {
            result = true;
            old_default = m_priv->m_default_interface;
            m_priv->m_default_interface = iter->second;
        }
    }

    if( result ) { m_priv->m_signal_default_interface_changed.emit( old_default, m_priv->m_default_interface ); }

    return result;
}

bool Object::set_default_interface( std::shared_ptr<Interface> interface_ptr ) {
    if( !interface_ptr ) { return false; }

    m_priv->m_default_interface = interface_ptr;
    return true;
}

void Object::remove_default_interface() {
    if( !m_priv->m_default_interface ) { return; }

    std::shared_ptr<Interface> old_default = m_priv->m_default_interface;
    m_priv->m_default_interface = std::shared_ptr<Interface>();
    m_priv->m_signal_default_interface_changed.emit( old_default, m_priv->m_default_interface );
}

const Object::Children& Object::children() const {
    return m_priv->m_children;
}

std::shared_ptr<Object> Object::child( const std::string& name ) const {
    Children::const_iterator i = m_priv->m_children.find( name );

    if( i == m_priv->m_children.end() ) { return std::shared_ptr<Object>(); }

    return i->second;
}

bool Object::add_child( const std::string& name, std::shared_ptr<Object> child, bool force ) {
    if( !child ) { return false; }

    if( !force && this->has_child( name ) ) { return false; }

    std::shared_ptr<Connection> conn = connection().lock();

    if( conn ) {
        m_priv->m_children[name] = child;
        child->set_connection( conn );

        if( m_priv->m_has_object_manager ){

        }

        return true;
    }

    return false;
}

bool Object::remove_child( const std::string& name ) {
    Children::iterator i = m_priv->m_children.find( name );

    if( i == m_priv->m_children.end() ) { return false; }

    m_priv->m_children.erase( i );
    return true;
}

bool Object::has_child( const std::string& name ) const {
    return m_priv->m_children.find( name ) != m_priv->m_children.end();
}

bool Object::has_child( std::shared_ptr<Object> child ) const {
    for( auto const& pair : m_priv->m_children ){
        if( pair.second == child ){
            return true;
        }
    }

    return false;
}

std::string Object::introspect( int space_depth ) const {
    std::ostringstream sout;
    std::string spaces;
    Interfaces::const_iterator i;
    Children::const_iterator c;

    for( int i = 0; i < space_depth; i++ ) { spaces += " "; }

    sout << spaces << "<node name=\"" << this->path() << "\">\n";

    if( !m_priv->m_is_lightweight ){
        sout << spaces << "  <interface name=\"" << DBUS_CXX_INTROSPECTABLE_INTERFACE << "\">\n"
        << spaces << "    <method name=\"Introspect\">\n"
        << spaces << "      <arg name=\"data\" type=\"s\" direction=\"out\"/>\n"
        << spaces << "    </method>\n"
        << spaces << "  </interface>\n"
        << spaces << "  <interface name=\"" << DBUS_CXX_PEER_INTERFACE << "\">\n"
        << spaces << "    <method name=\"Ping\" />\n"
        << spaces << "    <method name=\"GetMachineId\">\n"
        << spaces << "      <arg name=\"machine_uuid\" type=\"s\" direction=\"out\"/>\n"
        << spaces << "    </method>\n"
        << spaces << "  </interface>\n"
        << spaces << "  <interface name=\"" << DBUS_CXX_PROPERTIES_INTERFACE << "\">\n"
        << spaces << "    <method name=\"Get\">\n"
        << spaces << "        <arg type=\"s\" name=\"interface_name\" direction=\"in\"/>\n"
        << spaces << "        <arg type=\"s\" name=\"property_name\" direction=\"in\"/>\n"
        << spaces << "        <arg type=\"v\" name=\"value\" direction=\"out\"/>\n"
        << spaces << "      </method>\n"
        << spaces << "      <method name=\"GetAll\">\n"
        << spaces << "        <arg type=\"s\" name=\"interface_name\" direction=\"in\"/>\n"
        << spaces << "        <arg type=\"a{sv}\" name=\"properties\" direction=\"out\"/>\n"
        << spaces << "      </method>\n"
        << spaces << "      <method name=\"Set\">\n"
        << spaces << "        <arg type=\"s\" name=\"interface_name\" direction=\"in\"/>\n"
        << spaces << "        <arg type=\"s\" name=\"property_name\" direction=\"in\"/>\n"
        << spaces << "        <arg type=\"v\" name=\"value\" direction=\"in\"/>\n"
        << spaces << "      </method>\n"
        << spaces << "      <signal name=\"PropertiesChanged\">\n"
        << spaces << "       <arg type=\"s\" name=\"interface_name\"/>\n"
        << spaces << "        <arg type=\"a{sv}\" name=\"changed_properties\"/>\n"
        << spaces << "        <arg type=\"as\" name=\"invalidated_properties\"/>\n"
        << spaces << "      </signal>\n";
        if( m_priv->m_has_object_manager ){
            sout << spaces << "  <interface name=\"" << DBUS_CXX_OBJECT_MANAGER_INTERFACE << "\">\n"
            << spaces << "    <method name=\"GetManagedObjects\">\n"
            << spaces << "        <arg type=\"a{oa{sa{sv}}}\" name=\"objpath_interfaces_and_properties\" direction=\"out\"/>\n"
            << spaces << "      </method>\n"
            << spaces << "      <signal name=\"InterfacesAdded\">\n"
            << spaces << "       <arg type=\"o\" name=\"object_path\"/>\n"
            << spaces << "        <arg type=\"a{sa{sv}}\" name=\"interfaces_and_properties\"/>\n"
            << spaces << "      </signal>\n"
            << spaces << "      <signal name=\"InterfacesRemoved\">\n"
            << spaces << "       <arg type=\"o\" name=\"object_path\"/>\n"
            << spaces << "        <arg type=\"as\" name=\"interfaces\"/>\n"
            << spaces << "      </signal>\n";
        }
        sout << spaces << "  </interface>\n";

        for( i = m_priv->m_interfaces.begin(); i != m_priv->m_interfaces.end(); i++ ) {
            sout << i->second->introspect( space_depth + 2 );
        }
    }

    for( c = m_priv->m_children.begin(); c != m_priv->m_children.end(); c++ ) {
        sout << spaces << "  <node name=\"" << c->first << "\"/>\n";
    }

    sout << spaces << "</node>\n";
    return sout.str();
}

sigc::signal< void( std::shared_ptr<Interface> ) > Object::signal_interface_added() {
    return m_priv->m_signal_interface_added;
}

sigc::signal< void( std::shared_ptr<Interface> )> Object::signal_interface_removed() {
    return m_priv->m_signal_interface_removed;
}

sigc::signal< void( std::shared_ptr<Interface>, std::shared_ptr<Interface> )> Object::signal_default_interface_changed() {
    return m_priv->m_signal_default_interface_changed;
}

HandlerResult Object::handle_message( std::shared_ptr<const Message> message ) {
    std::shared_ptr<Connection> conn = connection().lock();
    std::shared_ptr<const CallMessage> msg;

    if( !conn ) {
        SIMPLELOGGER_ERROR( LOGGER_NAME, "Object::handle_call_message: unable to handle call message: invalid connection" );
        return HandlerResult::Not_Handled;
    }

    if( message->type() != MessageType::CALL ) {
        SIMPLELOGGER_ERROR( LOGGER_NAME, "Object::handle_call_message: unable to handle call message: message is not call message" );
        return HandlerResult::Not_Handled;
    }

    msg = std::static_pointer_cast<const CallMessage>( message );

    if( msg->interface_name() == DBUS_CXX_INTROSPECTABLE_INTERFACE ) {
        SIMPLELOGGER_DEBUG( LOGGER_NAME, "Object::handle_call_message: introspection interface called" );
        std::shared_ptr<ReturnMessage> return_message = msg->create_reply();
        std::string introspection = DBUSCXX_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE;
        introspection += this->introspect();
        *return_message << introspection;
        conn << return_message;
        return HandlerResult::Handled;
    } else if( msg->interface_name() == DBUS_CXX_PEER_INTERFACE ) {
        SIMPLELOGGER_DEBUG( LOGGER_NAME, "Object::handle_call_message: peer interface called" );

        if( msg->member() == "Ping" ) {
            conn << msg->create_reply();
            return HandlerResult::Handled;
        } else if( msg->member() == "GetMachineId" ) {
            std::ifstream inputFile( "/var/lib/dbus/machine-id" );
            std::string line;
            std::getline( inputFile, line );

            if( inputFile.eof() ) {
                SIMPLELOGGER_ERROR( LOGGER_NAME, "Unable to read machine ID." );
                std::shared_ptr<ErrorMessage> errmsg = msg->create_error_reply();
                errmsg->set_name( DBUSCXX_ERROR_IO_ERROR );
                errmsg->set_message( "Unable to read DBus machine ID file" );
                conn << errmsg;
                return HandlerResult::Handled;
            }

            std::shared_ptr<ReturnMessage> return_message = msg->create_reply();
            return_message << line;
            conn << return_message;
            return HandlerResult::Handled;
        }

        return HandlerResult::Invalid_Method;
    } else if( msg->interface_name() == DBUS_CXX_PROPERTIES_INTERFACE ){
        SIMPLELOGGER_DEBUG( LOGGER_NAME, "Object::handle_call_message: properties interface called" );
        std::string requestedInterfaceName;
        msg >> requestedInterfaceName;

        std::shared_lock lock( m_priv->m_interfaces_rwlock );
        Interfaces::iterator iface_iter = m_priv->m_interfaces.find( requestedInterfaceName);
        std::shared_ptr<Interface> interface_ptr;
        if( iface_iter == m_priv->m_interfaces.end() ) {
            // Unable to find an interface to use, try to use the default
            if( m_priv->m_default_interface ) {
                return m_priv->m_default_interface->handle_properties_message( conn, msg );
            }

            return  HandlerResult::Invalid_Interface;
        } else {
            interface_ptr = iface_iter->second;
            return interface_ptr->handle_properties_message( conn, msg );
        }
    } else if( msg->interface_name() == DBUS_CXX_OBJECT_MANAGER_INTERFACE ){
        SIMPLELOGGER_DEBUG( LOGGER_NAME, "Object::handle_call_message: object manager interface called" );

        if( msg->member() == "GetManagedObjects" ) {
            std::shared_ptr<ReturnMessage> reply = msg->create_reply();
            ObjectManagerObjects ret;
            for( Children::iterator c = m_priv->m_children.begin(); c != m_priv->m_children.end(); c++ ) {
                c->second->handle_objectmanager( &ret );
            }
            reply << ret;
            conn << reply;
            return HandlerResult::Handled;
        }

        return HandlerResult::Invalid_Method;
    }
    std::shared_lock lock( m_priv->m_interfaces_rwlock );
    Interfaces::iterator iface_iter;
    std::shared_ptr<Interface> interface_ptr;

    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Object::handle_message: message is good (it's a call message) for interface '" << msg->interface_name() << "'" );

    iface_iter = m_priv->m_interfaces.find( msg->interface_name() );

    /*
     * DBus Specification:
     *
     * In the absence of an INTERFACE field, if two or more interfaces on the same object
     * have a method with the same name, it is undefined which of those methods will be
     * invoked. Implementations may choose to either return an error, or deliver the message
     * as though it had an arbitrary one of those interfaces.
     */
    if( iface_iter == m_priv->m_interfaces.end() ) {
        // Unable to find an interface to use, try to use the default
        if( m_priv->m_default_interface ) {
            return m_priv->m_default_interface->handle_call_message( conn, msg );
        }

        return  HandlerResult::Invalid_Interface;
    } else {
        interface_ptr = iface_iter->second;
        return interface_ptr->handle_call_message( conn, msg );
    }
}

void Object::set_handling_thread( std::thread::id thr ){
    m_priv->m_calling_thread = thr;
}

std::thread::id Object::handling_thread(){
    return m_priv->m_calling_thread;
}

bool Object::is_lightweight() const{
    return m_priv->m_is_lightweight;
}

std::map<std::string,std::map<std::string,DBus::Variant>> Object::interfaces_and_properties(){
    std::map<std::string,std::map<std::string,DBus::Variant>> interfaces_and_properties;

    {
        std::shared_lock lock( m_priv->m_interfaces_rwlock );
        for( std::pair<std::string,std::shared_ptr<Interface>> pair : m_priv->m_interfaces ){
            std::map<std::string, DBus::Variant> properties;
            const std::set<std::shared_ptr<PropertyBase>> props = pair.second->properties();
            for( std::shared_ptr<PropertyBase> prop : props ){
                properties[ prop->name() ] = prop->variant_value();
            }
            interfaces_and_properties[ pair.first ] = properties;
        }
    }

    return interfaces_and_properties;
}

void Object::handle_objectmanager( ObjectManagerObjects* ret ){
    (*ret)[ m_priv->m_path ] = interfaces_and_properties();

    for( Children::iterator c = m_priv->m_children.begin(); c != m_priv->m_children.end(); c++ ) {
        c->second->handle_objectmanager( ret );
    }
}

bool Object::has_objectmanager() const{
    return m_priv->m_has_object_manager;
}

void Object::set_has_objectmanager( bool objectmanager ){
    m_priv->m_has_object_manager = objectmanager;

    if( objectmanager ){
        // Go through all of our children, making sure that when an interface is
        // added or removed the correct signal is emitted onto the bus
        for( Children::iterator c = m_priv->m_children.begin(); c != m_priv->m_children.end(); c++ ) {
            c->second->add_self_to_objectmanager( this );
        }
    }else{
        for( sigc::connection conn : m_priv->m_interface_added_connections ){
            conn.disconnect();
        }
        for( sigc::connection conn : m_priv->m_interface_removed_connections ){
            conn.disconnect();
        }

        m_priv->m_interface_added_connections.clear();
        m_priv->m_interface_removed_connections.clear();
    }
}

void Object::add_self_to_objectmanager( DBus::Object* obj_manager ){
    sigc::connection added_con = signal_interface_added().connect( sigc::mem_fun( *obj_manager, &Object::child_interface_added ) );
    obj_manager->m_priv->m_interface_added_connections.push_back( added_con );
    sigc::connection removed_con = signal_interface_removed().connect( sigc::mem_fun( *obj_manager, &Object::child_interface_removed ) );
    obj_manager->m_priv->m_interface_removed_connections.push_back( removed_con );

    for( Children::iterator c = m_priv->m_children.begin(); c != m_priv->m_children.end(); c++ ) {
        c->second->add_self_to_objectmanager( obj_manager );
    }
}

void Object::child_interface_removed( std::shared_ptr<DBus::Interface> iface ){
    std::vector<std::string> removed_interfaces;
    removed_interfaces.push_back( iface->name() );


    std::shared_ptr<DBus::SignalMessage> sig =
            DBus::SignalMessage::create( m_priv->m_path,
                                         DBUS_CXX_OBJECT_MANAGER_INTERFACE,
                                         "InterfacesRemoved" );

    sig << iface->path() << removed_interfaces;
    std::shared_ptr<Connection> conn = m_priv->m_connection.lock();
    if( conn ){
        conn << sig;
    }
}

void Object::child_interface_added( std::shared_ptr<DBus::Interface> iface ){
    std::map<std::string,std::map<std::string,DBus::Variant>> interfaces_and_properties;
    std::map<std::string, DBus::Variant> properties;

    const std::set<std::shared_ptr<PropertyBase>> props = iface->properties();
    for( std::shared_ptr<PropertyBase> prop : props ){
        properties[ prop->name() ] = prop->variant_value();
    }
    interfaces_and_properties[ iface->path() ] = properties;

    std::shared_ptr<DBus::SignalMessage> sig =
            DBus::SignalMessage::create( m_priv->m_path,
                                         DBUS_CXX_OBJECT_MANAGER_INTERFACE,
                                         "InterfacesAdded" );

    sig << iface->path() << interfaces_and_properties;
    std::shared_ptr<Connection> conn = m_priv->m_connection.lock();
    if( conn ){
        conn << sig;
    }
}

}

