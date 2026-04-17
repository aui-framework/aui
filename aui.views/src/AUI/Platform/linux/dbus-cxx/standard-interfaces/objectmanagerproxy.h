// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2025 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_OBJECTMANAGER_PROXY_H
#define DBUSCXX_OBJECTMANAGER_PROXY_H

#include "../interfaceproxy.h"
#include <memory>
#include <map>
#include <vector>
#include <string>
#include "../variant.h"

namespace DBus {

class ObjectManagerProxy : public InterfaceProxy {
private:
    ObjectManagerProxy();

public:
    static std::shared_ptr<ObjectManagerProxy> create();

    ObjectManagerObjects GetManagedObjects();

    DBus::SignalProxy<void(Path, std::map<std::string,std::map<std::string,DBus::Variant>>)>& signal_InterfacesAdded();

    DBus::SignalProxy<void(Path, std::vector<std::string> interfaces)>& signal_InterfacesRemoved();

private:
    std::shared_ptr<MethodProxy<ObjectManagerObjects()>> m_get_managed_objects_method;
    std::shared_ptr<DBus::SignalProxy<void(Path, std::map<std::string,std::map<std::string,DBus::Variant>>)>> m_signal_interfaces_added;
    std::shared_ptr<DBus::SignalProxy<void(Path, std::vector<std::string>)>> m_signal_interfaces_removed;
};

}

#endif // DBUSCXX_OBJECTMANAGER_PROXY_H
