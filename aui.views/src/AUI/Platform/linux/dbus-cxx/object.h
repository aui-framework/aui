// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *   Copyright (C) 2019 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/signal.h>
#include <AUI/Platform/linux/dbus-cxx/interface.h>
#include <AUI/Platform/linux/dbus-cxx/property.h>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <string>
#include "enums.h"

#ifndef DBUSCXXOBJECT_H
#define DBUSCXXOBJECT_H

namespace DBus {
class Connection;
class Interface;
class Message;
template <typename T_type> class Method;

/**
 * An Object represents a local object that is able to be called over the DBus.
 * Interfaces, methods, and signals can be created from this Object.
 *
 * Note: Because the header 'INTERFACE' is <b>not required</b> for method calls,
 * use the method \c set_default_interface to set an interface that will handle
 * method calls without 'INTERFACE' set.  If a message is received without the
 * 'INTERFACE' set, the following is used to determine how to handle the message:
 * - If the default interface is not set, return an error
 *  \c org.freedesktop.DBus.Error.UnknownInterface
 * - If the default interface is set, use that interface to try and handle the message.
 *  The normal message handling rules apply at this point, e.g. if the method does not
 *  exist an error \c org.freedesktop.DBus.Error.UnknownMethod will be returned.
 *
 * @ingroup local
 * @ingroup objects
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class Object {
protected:

    /**
     * This class has a protected constructor. Use the \c create() methods
     * to obtain a smart pointer to a new instance.
     */
    Object( const std::string& path );

public:
    /**
     * Typedef to the storage structure for an \c Object instance's
     * interfaces.
     *
     * \b Key - interface name
     * \b Value -smart pointer to an interface.
     *
     * Can access \e type as \c Object::Interfaces
     */
    typedef std::map<std::string, std::shared_ptr<Interface> > Interfaces;

    /**
     * Typedef to storage structure for an \c Object instance's
     * children. The \e key is the child's name and the \e value
     *
     * \b Data \b Structure - map: an object's children must be uniquely named
     * \b Key - child name
     * \b Value -smart pointer to a child object.
     *
     * Can access \e type as \c Object::Children
     */
    typedef std::map<std::string, std::shared_ptr<Object>> Children;

    /**
     * Create an Object that will be exported out onto the bus.  In general you should prefer to use
     * DBus::Connection::create_object which will set up the object for you.  If you do create the
     * object directly by calling create, you must then call DBus::Connection::register_object to
     * make sure that it can be called over the bus.
     *
     * @param path The path the object will handle.  This must be an absolute path.
     */
    static std::shared_ptr<Object> create( const std::string& path = std::string() );

    virtual ~Object();

    /**
     *  Returns the path this handler is associated with.  Note that this is always an absolute path.
     */
    const Path& path() const;

    /** Returns the connection this handler is registered with */
    std::weak_ptr<Connection> connection() const;

    /** Unregisters the handler */
    bool unregister();

    /** Emitted when this object is registered with a connection */
    sigc::signal<void( std::shared_ptr<Connection> ) >& signal_registered();

    /**
     * Emitted when this object is unregistered from a connection
     */
    sigc::signal<void( std::shared_ptr<Connection> ) >& signal_unregistered();

    /**
     * Sets the connection that this object is on.
     * Generally, you should not call this, as calling Connection::register_object
     * will do this for you automatically.
     *
     * @param conn
     */
    void set_connection( std::shared_ptr<Connection> conn );

    /** Get all the interfaces associated with this Object instance */
    const Interfaces& interfaces() const;

    /** Returns the interface with the given name */
    std::shared_ptr<Interface> interface_by_name( const std::string& name ) const;

    /** Adds the interface to this object */
    bool add_interface( std::shared_ptr<Interface> interface_by_name );

    /**
     * Creates and adds the named interface to this object.
     *
     * If the string is empty, and there is no default interface set, this will set the default interface.
     *
     * @return the newly created interface
     */
    std::shared_ptr<Interface> create_interface( const std::string& name );

    /**
     * Creates a method with a signature based on the @param slot parameter signature and adds it to the default interface
     * @return A smart pointer to the newly created method
     * @param slot This slot will be called with the given signature when the method is invoked
     *
     * Template parameters of the sigc::slot will determine the signature of
     * the method created.
     */
    template <typename T_type>
    std::shared_ptr<Method<T_type> >
    create_method( const std::string& method_name, sigc::slot<T_type> slot ) {
        if( !default_interface() ) {
            this->create_interface( "" );
            this->set_default_interface( "" );
        }

        // TODO throw an error if the default interface still doesn't exist

        std::shared_ptr< Method<T_type> > method;
        method = default_interface()->create_method<T_type>( method_name );
        method->set_method( slot );
        return method;
    }

    /**
     * Creates a method with a signature based on the @param slot parameter signature and adds it to the named interface
     * @return A smart pointer to the newly created method
     * @param interface_name The DBus interface, in org.example.Foo style
     * @param method_name The name of the method that this is for.
     * @param slot This slot will be called with the given signature when the method is invoked
     *
     * Template parameters of the sigc::slot will determine the signature of
     * the method created.
     */
    template <typename T_type>
    std::shared_ptr<Method<T_type> >
    create_method( const std::string& interface_name, const std::string& method_name, sigc::slot<T_type> slot ) {
        std::shared_ptr<Interface> interface_ptr;
        interface_ptr = this->interface_by_name( interface_name );

        if( !interface_ptr ) { interface_ptr = this->create_interface( interface_name ); }

        // TODO throw an error if the interface still doesn't exist

        std::shared_ptr< Method<T_type> > method;
        method = interface_ptr->create_method<T_type>( method_name );
        method->set_method( slot );
        return method;
    }

    template <typename T_type>
    std::shared_ptr<Property<T_type>>
    create_property( const std::string& interface_name,
                     const std::string& property_name,
                     PropertyAccess access_type = PropertyAccess::ReadWrite,
                     PropertyUpdateType update_type = PropertyUpdateType::Updates ){
        std::shared_ptr<Interface> interface_ptr;
        interface_ptr = this->interface_by_name( interface_name );

        if( !interface_ptr ) { interface_ptr = this->create_interface( interface_name ); }

        return interface_ptr->create_property<T_type>( property_name, access_type, update_type );
    }

    /**
     * Removes the interface found with the given name.
     *
     * This will not remove the default interface if the name matches - use remove_default_interface instead
     */
    void remove_interface( const std::string& name );

    /**
     * Test whether an Object has a named interface
     * @return \c True if the object has the named interface, \c false otherwise
     * @param name The name of the interface to look for
     */
    bool has_interface( const std::string& name );

    /**
     * Get the default interface associated with this object
     * @return The default interface, or a null smart pointer if no default is set
     */
    std::shared_ptr<Interface> default_interface() const;

    /**
     * Set the default interface to a specific name
     *
     * The currently-existing interface with the given name is used.  If an
     * interface with the given name is not found, the default interface
     * will not be set.
     *
     * @param new_default_name The name of the interface to use as the default.
     * @return \c True if an interface with the specified name was found, \c false otherwise.
     */
    bool set_default_interface( const std::string& new_default_name );

    /**
     * Set the default interface for handling calls to no interface.  If the shared
     * pointer is invalid, this function returns false and does nothing
     *
     * @param interface True if the default interface was updated, false otherwise.
     */
    bool set_default_interface( std::shared_ptr<Interface> interface_by_name );

    /**
     * Removes the currently set (if any) default interface. There wil not
     * be a default interface set.
     */
    void remove_default_interface();

    /**
     * Creates a signal with a return value (possibly \c void ) and a variable number of
     * parameters and adds it to the default interface.
     *
     * <b>Note</b>: This will create a default interface if it does not already exist.
     *
     * Template parameters for the \c create_signal() call will determine the
     * signature of the signal created.
     *
     * @return A smart pointer to the newly created signal
     */
    template <class... T_type>
    std::shared_ptr<Signal<T_type...> >
    create_signal( const std::string& name ) {
        std::shared_ptr<DBus::Signal<T_type...> > sig;
        std::shared_ptr<Interface> iface = this->default_interface();

        if( !iface ) { iface = this->create_interface( "" ); }

        sig = iface->create_signal<T_type...>( name );
        return sig;
    }

    /**
     * Creates a signal with a return value (possibly \c void ) and a variable number of
     * parameters and adds it to the named interface
     *
     * Template parameters for the \c create_signal() call will determine the
     * signature of the signal created.
     *
     * @return A smart pointer to the newly created signal
     */
    template <class... T_type>
    std::shared_ptr<Signal<T_type...> >
    create_signal( const std::string& iface, const std::string& name ) {
        std::shared_ptr<DBus::Signal<T_type...> > sig;

        if( !has_interface( iface ) ) { this->create_interface( iface ); }

        sig = this->interface_by_name( iface )->create_signal<T_type...>( name );
        return sig;
    }

    /** Get the children associated with this object instance */
    const Children& children() const;

    /**
     * Get a named child of this object
     * @return A smart pointer to a child with the specified name, or a null smart pointer if no child found.
     * @param name The name of the child to return
     */
    std::shared_ptr<Object> child( const std::string& name ) const;

    /**
     * Add an object as a child with a specified name
     * This method will fail if the object already has a child with the
     * specified name and \c force is not set.
     * @return \c True if the child was successfully added, \c false otherwise.
     * @param name The name to use for the child.
     * @param child A smart pointer to an object to add as a child.
     * @param force If \c true a child with the same name will be replaced.
     */
    bool add_child( const std::string& name, std::shared_ptr<Object> child, bool force = false );

    /**
     * Remove the named child from this object
     * @return \c True if the child was found and removed, \c false if no child by the name was found to remove.
     * @param name The name of the child to remove.
     */
    bool remove_child( const std::string& name );

    /**
     * Test whether an object has a child with a specified name
     * @return \c True if the object has the named child, \c false otherwise.
     * @param name The name of the child to test.
     */
    bool has_child( const std::string& name ) const;
    bool has_child( std::shared_ptr<Object> child ) const;

    /** Returns a DBus XML description of this interface */
    std::string introspect( int space_depth = 0 ) const;

    /**
     * Signal emitted when an interface is added to this object.
     *
     * The first parameter of the callback is a pointer to the newly added interface.
     */
    sigc::signal<void( std::shared_ptr<Interface> ) > signal_interface_added();

    /**
     * Signal emitted when an interface is removed from this object.
     *
     * The first parameter of the callback is a pointer to the removed interface.
     */
    sigc::signal<void( std::shared_ptr<Interface> ) > signal_interface_removed();

    /**
     * Signal emitted when the default interface of this object is changed.
     *
     * The first parameter of the callback is a pointer to the old default
     * interface, and the second parameter is a callback to the new default
     * interface.
     */
    sigc::signal<void( std::shared_ptr<Interface> /*old default*/, std::shared_ptr<Interface> /*new default*/ )> signal_default_interface_changed();

    /**
     * Handles the specified message on the specified connection
     *
     * If \c msg is an introspection message, the object will rely on its
     * \c introspection() method to provide a reply.
     *
     * Looks for interfaces specified in the message first. If the message
     * does not specify an interface or the specified interface is not found
     * the default interface will be used.
     *
     * If the specified interface is found, all matching interfaces will be
     * tried until an interface handler returns \c HandlerResult::HANDLED. Once an interface
     * returns \c HandlerResult::HANDLED no further interfaces will be tried.
     *
     * If no interface returns \c HandlerResult::HANDLED the default interface (if one is
     * set) will be tried.
     *
     * @return \c HandlerResult::HANDLED if this object handled the message, \c HandlerResult::NOT_HANDLED otherwise
     * @param conn The Connection to send the reply message on
     * @param msg The message to handle; must be a CallMessage or it will not be handled
     */
    HandlerResult handle_message( std::shared_ptr<const Message> msg );

    void set_handling_thread( std::thread::id thr );
    std::thread::id handling_thread();

    bool is_lightweight() const;

    static std::shared_ptr<Object> create_lightweight( const std::string& path = std::string() );

    bool has_objectmanager() const;

    void set_has_objectmanager( bool objectmanager );

private:
    void handle_objectmanager( ObjectManagerObjects* ret );
    std::map<std::string,std::map<std::string,DBus::Variant>> interfaces_and_properties();
    void child_interface_removed( std::shared_ptr<DBus::Interface> iface );
    void child_interface_added( std::shared_ptr<DBus::Interface> iface );
    void add_self_to_objectmanager( DBus::Object* obj_manager );

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;
};

}
#endif
