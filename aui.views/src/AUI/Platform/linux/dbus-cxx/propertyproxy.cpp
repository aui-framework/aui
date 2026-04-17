// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/propertyproxy.h>
#include <AUI/Platform/linux/dbus-cxx/variant.h>
#include <AUI/Platform/linux/dbus-cxx/interfaceproxy.h>
#include <AUI/Platform/linux/dbus-cxx/objectproxy.h>
#include "property.h"

using DBus::PropertyProxyBase;

class PropertyProxyBase::priv_data {
public:
    priv_data( std::string name, PropertyUpdateType update) :
        m_name( name ),
        m_propertyUpdate( update ),
        m_interface( nullptr ),
        m_valueSet( false )
    {}

    std::string m_name;
    PropertyUpdateType m_propertyUpdate;
    sigc::signal<void(DBus::Variant)> m_propertyChangedSignal;
    InterfaceProxy* m_interface;
    Variant m_value;
    bool m_valueSet;
};

PropertyProxyBase::PropertyProxyBase( std::string name, PropertyUpdateType update ) :
    m_priv( std::make_unique<priv_data>( name, update ) ) {

}

PropertyProxyBase::~PropertyProxyBase(){}

std::string PropertyProxyBase::name() const {
    return m_priv->m_name;
}

DBus::Variant PropertyProxyBase::variant_value() {
    if( !m_priv->m_valueSet && m_priv->m_interface ){
        std::shared_ptr<CallMessage> msg =
                CallMessage::create( m_priv->m_interface->object()->destination(),
                                     m_priv->m_interface->path(),
                                     DBUS_CXX_PROPERTIES_INTERFACE,
                                     "Get" );
        msg << m_priv->m_interface->name() << m_priv->m_name;

        std::shared_ptr<const ReturnMessage> ret =
                m_priv->m_interface->call( msg );

        MessageIterator iter = ret->begin();
        Variant var;
        iter >> var;

        updated_value( var );
    }

    return m_priv->m_value;
}

DBus::PropertyUpdateType PropertyProxyBase::update_type() const {
    return m_priv->m_propertyUpdate;
}

sigc::signal<void(DBus::Variant)> PropertyProxyBase::signal_generic_property_changed() {
    return m_priv->m_propertyChangedSignal;
}

void PropertyProxyBase::set_value( DBus::Variant value ) {
    if( m_priv->m_propertyUpdate == PropertyUpdateType::Const ){
        // We can't set the value for this
        return;
    }

    // Create a CallMessage to set the value on the remote object.
    std::shared_ptr<CallMessage> msg =
            CallMessage::create( m_priv->m_interface->object()->destination(),
                                 m_priv->m_interface->path(),
                                 "org.freedesktop.DBus.Properties",
                                 "Set" );
    msg << m_priv->m_interface->name() << m_priv->m_name << value;

    std::shared_ptr<const ReturnMessage> ret =
            m_priv->m_interface->call( msg );

    // This is probably not needed, as the remote object will likely emit a signal with the new value,
    // but this shouldn't cause an issue.
    m_priv->m_value = value;
}

void PropertyProxyBase::set_interface( InterfaceProxy *proxy ){
    m_priv->m_interface = proxy;
}

DBus::InterfaceProxy* PropertyProxyBase::interface_name() const {
    return m_priv->m_interface;
}

void PropertyProxyBase::updated_value(Variant value){
    m_priv->m_value = value;
    m_priv->m_valueSet = true;

    m_priv->m_propertyChangedSignal.emit( m_priv->m_value );
}

void PropertyProxyBase::invalidate(){
    m_priv->m_valueSet = false;
}
