// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2019 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/methodproxybase.h>
#include <AUI/Platform/linux/dbus-cxx/signalproxy.h>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include "path.h"
#include <AUI/Platform/linux/dbus-cxx/propertyproxy.h>

#ifndef DBUSCXX_INTERFACEPROXY_H
#define DBUSCXX_INTERFACEPROXY_H

namespace DBus {

class CallMessage;
class Connection;
class ObjectProxy;
class PendingCall;
class ReturnMessage;
template <typename signature> class MethodProxy;

/**
 * An InterfaceProxy represents a remote Interface in another application on the DBus.
 *
 * Note that the interface name is immutable and cannot be changed once the interface
 * has been created.
 *
 * @ingroup objects
 * @ingroup proxy
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class InterfaceProxy {
protected:
    InterfaceProxy( const std::string& name );

public:
    typedef std::multimap<std::string, std::shared_ptr<MethodProxyBase>> Methods;

    typedef std::set<std::shared_ptr<SignalProxyBase>> Signals;

    static std::shared_ptr<InterfaceProxy> create( const std::string& name = std::string() );

    virtual ~InterfaceProxy();

    ObjectProxy* object() const;

    Path path() const;

    std::weak_ptr<Connection> connection() const;

    const std::string& name() const;

    const Methods& methods() const;

    /** Returns the first method with the given name */
    std::shared_ptr<MethodProxyBase> method( const std::string& name ) const;

    template <class T_type>
    std::shared_ptr<MethodProxy<T_type>> create_method( const std::string& name ) {
        std::shared_ptr< MethodProxy<T_type> > method;
        method = MethodProxy<T_type>::create( name );

        if( !this->add_method( method ) ) {
            return std::shared_ptr< MethodProxy<T_type> >();
        }

        return method;
    }

    /**
     * Adds the named method.  If a method with the same name already exists,
     * does not replace the current method(returns false).
     */
    bool add_method( std::shared_ptr<MethodProxyBase> method );

    /** Removes the method with the given name */
    void remove_method( const std::string& name );

    /** Removed the specific method */
    void remove_method( std::shared_ptr<MethodProxyBase> method );

    /** True if the interface has a method with the given name */
    bool has_method( const std::string& name ) const;

    /** True if the interface has the specified method */
    bool has_method( std::shared_ptr<MethodProxyBase> method ) const;

    /**
     * Get the (local) cache of all properties.
     * @return
     */
    const std::map<std::string,std::shared_ptr<PropertyProxyBase>>& properties() const;

    std::shared_ptr<PropertyProxyBase> property( const std::string& name ) const;

    /** True if the interface has a property with the given name */
    bool has_property( const std::string& name ) const;

    /** True if the interface has the specified property */
    bool has_property( std::shared_ptr<PropertyProxyBase> property ) const;

    /** Removes the property with the given name */
    void remove_property( const std::string& name );

    /** Removed the specific property */
    void remove_property( std::shared_ptr<PropertyProxyBase> method );

    /**
     * Create a property with the given type, and the given name.
     */
    template <class T_type>
    std::shared_ptr<PropertyProxy<T_type>> create_property( const std::string& name,
                                                            PropertyUpdateType update = PropertyUpdateType::Updates ){
        std::shared_ptr< PropertyProxy<T_type> > prop;
        prop = PropertyProxy<T_type>::create( name, update );

        if( !this->add_property( prop ) ) {
            return std::shared_ptr< PropertyProxy<T_type> >();
        }

        return prop;
    }

    /**
     * Adds the given property.  If a property with the same name already exists,
     * does not replace the current property(returns false).
     */
    bool add_property( std::shared_ptr<PropertyProxyBase> property );

    /**
     * Ask the remote object for the status of all of its properties.
     */
    void cache_properties();

    std::shared_ptr<CallMessage> create_call_message( const std::string& method_name ) const;

    std::shared_ptr<const ReturnMessage> call( std::shared_ptr<const CallMessage>, int timeout_milliseconds = -1 ) const;

    std::shared_ptr<const ReturnMessage> call_notimeout( std::shared_ptr<const CallMessage> ) const;

    //      std::shared_ptr<PendingCall> call_async( std::shared_ptr<const CallMessage>, int timeout_milliseconds=-1 ) const;

    template <class T_arg>
    std::shared_ptr<SignalProxy<T_arg >> create_signal( const std::string& sig_name ) {
        std::shared_ptr< SignalProxy<T_arg> > sig;
        SignalMatchRule match = MatchRuleBuilder::create()
            .set_path( this->path() )
            .set_interface( name() )
            .set_member( sig_name )
            .as_signal_match();
        sig = SignalProxy<T_arg>::create( match );
        this->add_signal( sig );
        return sig;
    }

    const Signals& signals() const;

    std::shared_ptr<SignalProxyBase> signal( const std::string& signame );

    bool add_signal( std::shared_ptr<SignalProxyBase> sig );

    bool remove_signal( const std::string& signame );

    bool remove_signal( std::shared_ptr<SignalProxyBase> sig );

    bool has_signal( const std::string& signame ) const;

    bool has_signal( std::shared_ptr<SignalProxyBase> sig ) const;

private:
    void on_object_set_path( const std::string& path );

    void set_object( ObjectProxy* obj );

    void property_updated( std::string,std::map<std::string,DBus::Variant>,std::vector<std::string> );

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;

    friend class ObjectProxy;
};

}

#endif
