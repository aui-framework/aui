// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "introspectableinterfaceproxy.h"

using DBus::IntrospectableInterfaceProxy;

IntrospectableInterfaceProxy::IntrospectableInterfaceProxy() :
    InterfaceProxy ( DBUS_CXX_INTROSPECTABLE_INTERFACE ){
    m_introspect_method = this->create_method<std::string()>( "Introspect" );
}

std::string IntrospectableInterfaceProxy::Introspect(){
    return (*m_introspect_method)();
}

std::shared_ptr<IntrospectableInterfaceProxy> IntrospectableInterfaceProxy::create(){
    return std::shared_ptr<IntrospectableInterfaceProxy>( new IntrospectableInterfaceProxy() );
}
