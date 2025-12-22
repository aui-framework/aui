// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/signalproxy.h>
#include <AUI/Platform/linux/dbus-cxx/interfaceproxy.h>
#include <AUI/Platform/linux/dbus-cxx/propertyproxy.h>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include "path.h"

#ifndef DBUSCXX_OBJECTPROXY_H
#define DBUSCXX_OBJECTPROXY_H

namespace DBus {

class Connection;
class CallMessage;
class InterfaceProxy;
class MethodProxyBase;
class PendingCall;
class ReturnMessage;
template <typename signature> class MethodProxy;
class PeerInterfaceProxy;
class IntrospectableInterfaceProxy;
class PropertiesInterfaceProxy;
class ObjectManagerProxy;

/**
 * Object proxies are local proxies that provide local methods and signals for
 * remote objects with dbus interfaces.
 *
 * @ingroup proxy
 * @ingroup objects
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class ObjectProxy {
protected:

    /**
     * This class has a protected constructor. Use the \c create() methods
     * to obtain a smart std::shared_ptr<ObjectProxy> to a new instance.
     */
    ObjectProxy( std::shared_ptr<Connection> conn, const std::string& destination, const std::string& path );

public:
    /**
     * Creates an ObjectProxy with a specific path
     * @param path The path of the object that this will be a proxy for
     */
    static std::shared_ptr<ObjectProxy> create( const std::string& path );

    /**
     * Creates an ObjectProxy with a specific path
     * @param destination The destination address that this will be a proxy for
     * @param path The path of the object that this will be a proxy for
     */
    static std::shared_ptr<ObjectProxy> create( const std::string& destination, const std::string& path );

    static std::shared_ptr<ObjectProxy> create( std::shared_ptr<Connection> conn, const std::string& path );

    static std::shared_ptr<ObjectProxy> create( std::shared_ptr<Connection> conn, const std::string& destination, const std::string& path );

    virtual ~ObjectProxy();

    std::weak_ptr<Connection> connection() const;

    void set_connection( std::shared_ptr<Connection> conn );

    const std::string& destination() const;

    void set_destination( const std::string& destination );

    const Path& path() const;

    void set_path( const std::string& path );

    typedef std::map<std::string, std::shared_ptr<InterfaceProxy>> Interfaces;

    const Interfaces& interfaces() const;

    /** Returns the first interface with the given name */
    std::shared_ptr<InterfaceProxy> interface_by_name( const std::string& name ) const;

    /** Adds the interface to this object */
    bool add_interface( std::shared_ptr<InterfaceProxy> interface_ptr );

    /**
     * Creates and adds the named interface to this object
     *
     * @return the newly created interface
     */
    std::shared_ptr<InterfaceProxy> create_interface( const std::string& name );

    /** Removes the first interface with the given name */
    void remove_interface( const std::string& name );

    /** Removes the given interface */
    void remove_interface( std::shared_ptr<InterfaceProxy> interface_ptr );

    bool has_interface( const std::string& name ) const;

    bool has_interface( std::shared_ptr<InterfaceProxy> interface_ptr ) const;

    /** Adds the method to the named interface */
    bool add_method( const std::string& interface_name, std::shared_ptr<MethodProxyBase> method );

    std::shared_ptr<CallMessage> create_call_message( const std::string& interface_name, const std::string& method_name ) const;

    std::shared_ptr<CallMessage> create_call_message( const std::string& method_name ) const;

    /**
     * Forwards this CallMessage to the Connection that this ObjectProxy is on, and returns a
     * message with the response.
     *
     * @param timeout_milliseconds
     * @return
     */
    std::shared_ptr<const ReturnMessage> call( std::shared_ptr<const CallMessage>, int timeout_milliseconds = -1 ) const;

    /**
     * A timeout-less version of @ref call().
     */
    std::shared_ptr<const ReturnMessage> call_notimeout( std::shared_ptr<const CallMessage> ) const;

    /**
     * Creates a proxy method with a signature based on the template parameters and adds it to the named interface
     * @return A smart pointer to the newly created method proxy
     * @param interface_name The name of the interface to add this proxy method to
     * @param method_name The name to assign to the newly create method proxy. This name will be used to perform the dbus-call.
     */
    template <class T_type>
    std::shared_ptr<MethodProxy<T_type>>
    create_method( const std::string& interface_name, const std::string& method_name ) {
        std::shared_ptr<InterfaceProxy> interface_ptr = this->interface_by_name( interface_name );

        if( !interface_ptr ) { interface_ptr = this->create_interface( interface_name ); }

        return interface_ptr->create_method<T_type>( method_name );
    }

    /**
     * Creates a signal proxy with a signature based on the template parameters and adds it to the named interface
     * @return A smart pointer to the newly created signal proxy
     * @param interface_name The name of the interface to add this proxy signal to
     * @param sig_name The name to assign to the newly created signal proxy.
     */
    template <class T_type>
    std::shared_ptr<SignalProxy<T_type> >
    create_signal( const std::string& interface_name, const std::string& sig_name ) {
        std::shared_ptr<InterfaceProxy> interface_ptr = this->interface_by_name( interface_name );

        if( !interface_ptr ) { interface_ptr = this->create_interface( interface_name ); }

        return interface_ptr->create_signal<T_type>( sig_name );
    }

    template <class T_type>
    std::shared_ptr<PropertyProxy<T_type>>
    create_property( const std::string& interface_name,
                     const std::string& property_name,
                     PropertyAccess access_type = PropertyAccess::ReadWrite,
                     PropertyUpdateType update = PropertyUpdateType::Updates ) {
        std::shared_ptr<InterfaceProxy> interface_ptr = this->interface_by_name( interface_name );

        if( !interface_ptr ) { interface_ptr = this->create_interface( interface_name ); }

        return interface_ptr->create_property<T_type>( property_name, update );
    }

    /**
     * Return a signal that you may connect to when an interface is added.
     * This will be emitted from the thread that added the interface.
     *
     * @return
     */
    sigc::signal<void( std::shared_ptr<InterfaceProxy> )> signal_interface_added();

    /**
     * Return a signal that you may connect to when an interface is removed.
     * This will be emitted from the thread that added the interface.
     *
     * @return
     */
    sigc::signal<void( std::shared_ptr<InterfaceProxy> )> signal_interface_removed();

    std::shared_ptr<PeerInterfaceProxy> getPeerInterface();

    std::shared_ptr<IntrospectableInterfaceProxy> getIntrospectableInterface();

    std::shared_ptr<PropertiesInterfaceProxy> getPropertiesInterface();

    std::shared_ptr<ObjectManagerProxy> getObjectManagerInterface();

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;
};

}

#endif
