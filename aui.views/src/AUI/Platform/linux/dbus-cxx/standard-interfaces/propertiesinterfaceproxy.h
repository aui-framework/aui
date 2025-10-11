// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_PROPERTIES_INTERFACE_PROXY_H
#define DBUSCXX_PROPERTIES_INTERFACE_PROXY_H

#include "../interfaceproxy.h"
#include <memory>
#include <string>
#include <map>

namespace DBus{

class Variant;

class PropertiesInterfaceProxy : public InterfaceProxy {
private:
    PropertiesInterfaceProxy();

public:
    static std::shared_ptr<PropertiesInterfaceProxy> create();

    Variant Get( std::string interface_name, std::string property_name );

    void Set( std::string interface_name, std::string property_name, Variant property_value );

    std::map<std::string,Variant> GetAll( std::string interface_name );

private:
    std::shared_ptr<MethodProxy<Variant(std::string,std::string)>> m_get_method;
    std::shared_ptr<MethodProxy<void(std::string,std::string,Variant)>> m_set_method;
    std::shared_ptr<MethodProxy<std::map<std::string,Variant>(std::string)>> m_getall_method;
};

}

#endif /* DBUSCXX_PROPERTIES_INTERFACE_PROXY_H */
