// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "peerinterfaceproxy.h"

using DBus::PeerInterfaceProxy;

PeerInterfaceProxy::PeerInterfaceProxy() :
    DBus::InterfaceProxy( DBUS_CXX_PEER_INTERFACE ){
    m_ping_method = this->create_method<void()>( "Ping" );
    m_get_machine_method = this->create_method<std::string()>( "GetMachineId" );
}

std::shared_ptr<PeerInterfaceProxy> PeerInterfaceProxy::create(){
    return std::shared_ptr<PeerInterfaceProxy>( new PeerInterfaceProxy() );
}


void PeerInterfaceProxy::Ping(){
    (*m_ping_method)();
}

std::string PeerInterfaceProxy::GetMachineId(){
    return (*m_get_machine_method)();
}
