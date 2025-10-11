// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_INTROPSECTION_INTERFACE_PROXY_H
#define DBUSCXX_INTROPSECTION_INTERFACE_PROXY_H

#include "../interfaceproxy.h"
#include <memory>
#include <string>

namespace DBus {

class IntrospectableInterfaceProxy : public InterfaceProxy {
private:
    IntrospectableInterfaceProxy();

public:
    static std::shared_ptr<IntrospectableInterfaceProxy> create();

    std::string Introspect();

private:
    std::shared_ptr<MethodProxy<std::string()>> m_introspect_method;
};

}

#endif /* DBUSCXX_INTROPSECTION_INTERFACE_PROXY_H */
