// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/property.h>
#include <AUI/Platform/linux/dbus-cxx/variant.h>
#include <AUI/Platform/linux/dbus-cxx/interface.h>
#include <AUI/Platform/linux/dbus-cxx/signalmessage.h>
#include "property.h"

using DBus::PropertyBase;

class PropertyBase::priv_data {
public:
    priv_data( std::string name, PropertyAccess access, PropertyUpdateType update ) :
        m_name( name ),
        m_propertyUpdate( update ),
        m_interface( nullptr ),
        m_access( access )
    {}

    std::string m_name;
    PropertyUpdateType m_propertyUpdate;
    sigc::signal<void(DBus::Variant)> m_propertyChangedSignal;
    Interface* m_interface;
    Variant m_value;
    PropertyAccess m_access;
};

PropertyBase::PropertyBase( std::string name, PropertyAccess access, PropertyUpdateType update ) :
    m_priv( std::make_unique<priv_data>( name, access, update ) ) {

}

PropertyBase::~PropertyBase(){

}

std::string PropertyBase::name() const {
    return m_priv->m_name;
}

DBus::Variant PropertyBase::variant_value() const {
    return m_priv->m_value;
}

DBus::PropertyUpdateType PropertyBase::update_type() const {
    return m_priv->m_propertyUpdate;
}

void PropertyBase::set_value( DBus::Variant value ) {
    m_priv->m_value = value;

    if( !m_priv->m_interface ){
        return;
    }

    m_priv->m_interface->property_updated( this );
}

DBus::PropertyAccess PropertyBase::access_type() const {
    return m_priv->m_access;
}

void PropertyBase::setInterface( Interface* iface ){
    m_priv->m_interface = iface;
}
