// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/enums.h>
#include <AUI/Platform/linux/dbus-cxx/variant.h>
#include <memory>

#ifndef DBUSCXX_PROPERTYPROXY_H
#define DBUSCXX_PROPERTYPROXY_H
namespace DBus {

class InterfaceProxy;

/**
 * Base type of PropertyProxy to allow for storage in e.g. a vector.
 */
class PropertyProxyBase {
protected:
    PropertyProxyBase( std::string name, PropertyUpdateType update );

    ~PropertyProxyBase();

public:

    /**
     * Get the name of this propery.
     * @return
     */
    std::string name() const;

    /**
     * Get the value of this property as a Variant.
     *
     * If the value is stale, this will go and query the latest value from the
     * remote object.
     *
     * @return
     */
    Variant variant_value();

    PropertyUpdateType update_type() const;

    /**
     * This signal is emitted whenever the property changes
     * @return
     */
    sigc::signal<void(Variant)> signal_generic_property_changed();

    /**
     * Set the value of this property.
     *
     * When used on a remote property(a proxy), this will attempt to set
     * the value on the remote object.  If the property is READONLY, this
     * acts as a No-op.
     *
     * When used on a local property(adapter), this will emit the PropertyChanged
     * DBus signal in order to notify clients that the property has updated.
     * Note that the exact value of the PropertyUpdateType will determine what is
     * emitted(invalidated, new value, or invalidation)
     *
     * @param value The new value to set
     */
    void set_value( Variant value );

    InterfaceProxy* interface_name() const;

private:
    void set_interface( InterfaceProxy* proxy );
    void updated_value( Variant value );
    void invalidate();

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;

    // Declare InterfaceProxy as a friend so that it can set the interface
    friend class InterfaceProxy;
};

/**
 * Represents a remote DBus property.
 *
 * Properties can be Read, Write, or Readonly.
 */
template <typename T_type>
class PropertyProxy : public PropertyProxyBase {
private:
    PropertyProxy( std::string name, PropertyUpdateType update ) :
        PropertyProxyBase( name, update ) {
        signal_generic_property_changed().connect( sigc::mem_fun( *this, &PropertyProxy::parentUpdated ) );
    }

public:
    static std::shared_ptr<PropertyProxy<T_type>> create( std::string name, PropertyUpdateType update ) {
        return std::shared_ptr<PropertyProxy<T_type>>( new PropertyProxy<T_type>( name, update ) );
    }

    sigc::signal<void(T_type)> signal_property_changed() {
        return m_signal_changed;
    }

    void set_value( T_type t ) {
        PropertyProxyBase::set_value( Variant( t ) );
    }

    T_type value() {
        Variant v = variant_value();
        T_type t = v;
        return t;
    }

private:
    void parentUpdated( Variant v ){
        T_type t = v;
        m_signal_changed.emit( t );
    }

private:
    sigc::signal<void(T_type)> m_signal_changed;
};

} /* namespace DBus */

#endif /* DBUSCXX_PROPERTY_H */
