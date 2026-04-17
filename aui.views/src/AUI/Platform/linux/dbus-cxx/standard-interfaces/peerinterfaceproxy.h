// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_PEER_INTERFACE_PROXY_H
#define DBUSCXX_PEER_INTERFACE_PROXY_H

#include "../interfaceproxy.h"
#include <memory>

namespace DBus{

class PeerInterfaceProxy : public InterfaceProxy {
private:
    PeerInterfaceProxy();

public:
    static std::shared_ptr<PeerInterfaceProxy> create();

    void Ping();

    std::string GetMachineId();

private:
    std::shared_ptr<MethodProxy<void()>> m_ping_method;
    std::shared_ptr<MethodProxy<std::string()>> m_get_machine_method;
};


}

#endif /* DBUSCXX_PEER_INTERFACE_PROXY_H */
