// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009 by Rick L. Vinyard, Jr.                            *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "methodbase.h"
#include "dbus-cxx-private.h"
#include "dbus-cxx/connection.h"

namespace DBus {
class Message;

class MethodBase::priv_data {
public:
    priv_data( const std::string& name ) :
        m_name( name ) {}

    const std::string m_name;
    std::vector<std::string> m_arg_names;
};

MethodBase::MethodBase( const std::string& name ):
    m_priv( std::make_unique<priv_data>( name ) ) {
    SIMPLELOGGER_DEBUG( "DBus.MethodBase", "Creating new method with name " << name );
}

MethodBase::~MethodBase() {}

const std::string& MethodBase::name() const {
    return m_priv->m_name;
}

void MethodBase::set_arg_name( size_t i, const std::string& name ) {
    if( m_priv->m_arg_names.size() <= i ) {
        m_priv->m_arg_names.resize( i + 1 );
    }

    m_priv->m_arg_names[i] = name;
}

std::string MethodBase::arg_name( size_t i ) const {
    if( i < m_priv->m_arg_names.size() ) {
        return m_priv->m_arg_names[i];
    }

    return std::string();
}

uint32_t MethodBase::sendMessage( std::shared_ptr<Connection> connection, const std::shared_ptr<const Message> message ) {
    connection << message;
    return 0;
}

const std::vector<std::string>& MethodBase::arg_names() const {
    return m_priv->m_arg_names;
}

}

