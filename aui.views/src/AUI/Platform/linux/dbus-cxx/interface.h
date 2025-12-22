// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2019 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "enums.h"
#include "path.h"
#include <AUI/Platform/linux/dbus-cxx/methodbase.h>
#include <AUI/Platform/linux/dbus-cxx/signal.h>
#include <AUI/Platform/linux/dbus-cxx/property.h>
#include <set>
#include <map>
#include <mutex>
#include <shared_mutex>

#ifndef DBUSCXX_INTERFACE_H
#define DBUSCXX_INTERFACE_H

namespace DBus {
class CallMessage;
class Connection;
class Object;
class SignalBase;

/**
 * An Interface represents a local copy of a DBus interface.  A DBus interface is
 * an entry point that allows for object-orinted manipulation of local objects.
 * Interfaces contain two ore more elements separated by the period ('.') character.
 *
 * @ingroup objects
 * @ingroup local
 *
 * @todo Do something about the signals when the interface name is changed
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class Interface {
protected:
    /**
     * This class has a protected constructor. Use the \c create() methods
     * to obtain a smart pointer to a new instance.
     */
    Interface( const std::string& name );

public:
    /**
     * Typedef to the storage structure for methods.
     *
     * \b Data \b Structure - map
     * \b Key - method name
     * \b Value -smart pointer to a method.
     *
     * Can access \e type as \c Interface::Methods
     */
    typedef std::map<std::string, std::shared_ptr<MethodBase>> Methods;

    /**
     * Typedef to the storage structure for signals.
     *
     * \b Data \b Structure - sets is used since signal names are not needed for the interface, but must be unique.
     *
     * Can access \e type as \c Interface::Signals
     */
    typedef std::set<std::shared_ptr<SignalBase>> Signals;

    /**
     * Creates a named Interface
     * @param name The name of this interface
     */
    static std::shared_ptr<Interface> create( const std::string& name = std::string() );

    virtual ~Interface();

    /**
     * Handles the specified call message
     *
     * Looks for methods matching the name specified in the message, then
     * calls handle_call_message() for each matching message.
     *
     * Once a method returns \c HANDLED no further methods will be tried.
     *
     * @return \c HANDLED if one method in this interface handled the message, \c NOT_HANDLED otherwise
     * @param conn The Connection to send the reply message on
     * @param msg The CallMessage to handle
     */
    HandlerResult handle_call_message( std::shared_ptr<Connection> conn, std::shared_ptr<const CallMessage> message );

    /**
     * Handle the specified properties message.
     *
     * Depending on the type of properties message(Get/Set/GetAll), the appropriate response
     * will be built up and returned.
     *
     * @param conn The connection this is from
     * @param message The message with properties information
     * @return
     */
    HandlerResult handle_properties_message( std::shared_ptr<Connection> conn, std::shared_ptr<const CallMessage> message );

    /** Get the name of this interface */
    const std::string& name() const;

    /** Returns the path of the object associated with this interface or a null string if no object is associated */
    Path path() const;

    /** Returns the methods associated with this interface */
    const Methods& methods() const;

    /** Returns the method with the given name, or an invalid shared_ptr if not found */
    std::shared_ptr<MethodBase> method( const std::string& name ) const;

    /**
     * Creates a method with a return value (possibly \c void ) and $1 parameters
     * @return A smart pointer to the newly created method
     * @param name The name that will be associated with this method
     */
    template <typename T_type>
    std::shared_ptr<Method<T_type> >
    create_method( const std::string& name ) {
        std::shared_ptr< Method<T_type> > method;
        method = Method<T_type>::create( name );
        this->add_method( method );
        return method;
    }

    /**
     * Creates a method with a signature equivalent to the provided @param slot parameter's signature
     * @return A smart pointer to the newly created method
     * @param slot This slot will be called with the given signature when the method is invoked
     *
     * Template parameters of the sigc::slot will determine the signature of
     * the method created.
     */
    template <typename T_type>
    std::shared_ptr<Method<T_type> >
    create_method( const std::string& name, sigc::slot<T_type> slot ) {
        std::shared_ptr< Method<T_type> > method;
        method = Method<T_type>::create( name );
        method->set_method( slot );
        this->add_method( method );
        return method;
    }

    template <typename T_type>
    std::shared_ptr<Property<T_type>>
    create_property( const std::string& name,
                     PropertyAccess access_type = PropertyAccess::ReadWrite,
                     PropertyUpdateType update_type = PropertyUpdateType::Updates ) {
        std::shared_ptr<Property<T_type>> prop = Property<T_type>::create( name, access_type, update_type );
        add_property( prop );
        return prop;
    }

    bool add_property( std::shared_ptr<PropertyBase> prop );

    bool has_property( const std::string& name ) const;

    /** Adds the named method */
    bool add_method( std::shared_ptr<MethodBase> method );

    /** Removes the first method with the given name */
    bool remove_method( const std::string& name );

    /**
     * Removes the given method, if found.
     * @param method
     */
    bool remove_method( std::shared_ptr<MethodBase> method );

    /** True if the interface has a method with the given name */
    bool has_method( const std::string& name ) const;

    /**
     * Adds the given signal
     * @return true if the signal was successfully added, false otherwise
     */
    bool add_signal( std::shared_ptr<SignalBase> signal );

    /**
     * Removes the given signal
     * @return true if the signal was successfully removed, false otherwise
     *
     * One reason a signal couldn't be removed is if it wasn't a part of the interface.
     */
    bool remove_signal( std::shared_ptr<SignalBase> signal );

    /**
     * Removes all signals with the given name
     */
    bool remove_signal( const std::string& name );

    /**
     * True if the given signal is part of this interface
     */
    bool has_signal( std::shared_ptr<SignalBase> signal ) const;

    /** True if this interface has at least one signal with the given name */
    bool has_signal( const std::string& name ) const;

    /**
     * Creates a signal with any number of parameters
     * @return A smart pointer to the newly created signal
     * @param name The name that will be associated with this signal
     */
    template <class... T_type>
    std::shared_ptr<Signal<T_type...> >
    create_signal( const std::string& member ) {
        std::shared_ptr<DBus::Signal<T_type...> > sig;
        sig = DBus::Signal<T_type...>::create( path(), name(), member );

        if( this->add_signal( sig ) ) { return sig; }

        return std::shared_ptr<DBus::Signal<T_type...> >();
    }

    /** Returns the signals associated with this interface */
    const Signals& signals();

    const std::set<std::shared_ptr<PropertyBase>>& properties();

    /**
     * Returns the first signal found with a matching name.
     * If more than one signal has a given name there are no guarantees
     * as to which signal will be returned.
     */
    std::shared_ptr<SignalBase> signal( const std::string& signal_name );

    /** Signal emitted when a method of the given name is added */
    sigc::signal<void( std::shared_ptr<MethodBase> )> signal_method_added();

    /** Signal emitted when a method of the given name is removed */
    sigc::signal<void( std::shared_ptr<MethodBase> )> signal_method_removed();

    /** Returns a DBus XML description of this interface */
    std::string introspect( int space_depth = 0 ) const;

private:
    void set_path( const std::string& new_path );
    void property_updated( DBus::PropertyBase* prop );
    void set_connection( std::weak_ptr<Connection> conn );

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;

    friend class Object;
    friend class PropertyBase;
};

} /* namespace DBus */

#endif /* DBUS_CXX_INTERFACE_H */
