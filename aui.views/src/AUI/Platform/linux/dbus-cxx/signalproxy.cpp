// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009 by Rick L. Vinyard, Jr.                            *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "signalproxy.h"
#include <AUI/Platform/linux/dbus-cxx/signalmessage.h>
#include "path.h"
#include "signalbase.h"

namespace DBus {

class SignalProxyBase::priv_data {
public:
    priv_data() {}

    std::string m_match_rule;
};

SignalProxyBase::SignalProxyBase( const SignalMatchRule& matchRule ):
    SignalBase( matchRule.path(), matchRule.dbus_interface(), matchRule.member() ),
    m_priv( std::make_unique<priv_data>() ) {
    m_priv->m_match_rule = matchRule.match_rule();
}

SignalProxyBase::~SignalProxyBase() {
}

HandlerResult SignalProxyBase::handle_signal( std::shared_ptr<const SignalMessage> msg ) {
    if( !this->matches( msg ) ) { return HandlerResult::Not_Handled; }

    return on_dbus_incoming( msg );
}

const std::string& SignalProxyBase::match_rule() const {
    return m_priv->m_match_rule;
}

bool SignalProxyBase::matches( std::shared_ptr<const SignalMessage> msg ) {
    if( !msg || !msg->is_valid() ) { return false; }

    if( !interface_name().empty() && interface_name() != msg->interface_name() ) { return false; }

    if( !name().empty() && name() != msg->member() ) { return false; }

    if( !sender().empty() && sender() != msg->sender() ) { return false; }

    if( !destination().empty() && destination() != msg->destination() ) { return false; }

    if( !path().empty() && path() != msg->path() ) { return false; }

    return true;
}

void SignalProxyBase::update_match_rule(){
    m_priv->m_match_rule = MatchRuleBuilder::create()
            .set_path( path() )
            .set_interface( interface_name() )
            .set_member( name() )
            .as_signal_match()
            .match_rule();
}

}

