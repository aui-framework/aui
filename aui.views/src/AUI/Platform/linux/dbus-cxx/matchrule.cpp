// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "matchrule.h"

#include <string>

using DBus::MatchRuleBuilder;
using DBus::MatchRule;
using DBus::SignalMatchRule;
using DBus::MethodCallMatchRule;
using DBus::MethodReturnMatchRule;
using DBus::ErrorMatchRule;

/***************************************************************************/
class DBus::MatchRuleData {
public:
    MatchRuleData(){}

    std::string m_type;
    std::string m_path;
    std::string m_interface;
    std::string m_member;
    std::string m_sender;
    std::string m_destination;
};


/***************************************************************************/
MatchRuleBuilder::MatchRuleBuilder() :
    m_priv( std::make_unique<MatchRuleData>() )
{}

MatchRuleBuilder& MatchRuleBuilder::set_path( const std::string& path ) {
    m_priv->m_path = path;
    return *this;
}

MatchRuleBuilder& MatchRuleBuilder::set_interface( const std::string& interface_name ) {
    m_priv->m_interface = interface_name;
    return *this;
}

MatchRuleBuilder& MatchRuleBuilder::set_member( const std::string& member ) {
    m_priv->m_member = member;
    return *this;
}

MatchRuleBuilder& MatchRuleBuilder::set_sender( const std::string& sender ) {
    m_priv->m_sender = sender;
    return *this;
}

MatchRuleBuilder& MatchRuleBuilder::set_destination( const std::string& destination ) {
    m_priv->m_destination = destination;
    return *this;
}

SignalMatchRule MatchRuleBuilder::as_signal_match(){
    SignalMatchRule sig( m_priv );

    return sig;
}

MethodCallMatchRule MatchRuleBuilder::as_method_call_match(){
    MethodCallMatchRule meth( m_priv );

    return meth;
}

MethodReturnMatchRule MatchRuleBuilder::as_method_return_match(){
    MethodReturnMatchRule meth( m_priv );

    return meth;
}

ErrorMatchRule MatchRuleBuilder::as_error_match(){
    ErrorMatchRule err( m_priv );

    return err;
}

MatchRuleBuilder MatchRuleBuilder::create(){
    MatchRuleBuilder build;
    return build;
}

/***************************************************************************/
MatchRule::MatchRule( std::string type, std::shared_ptr<MatchRuleData> data ) :
    m_priv( data ){
    m_priv->m_type = type;
}

std::string MatchRule::path() const {
    return m_priv->m_path;
}

std::string MatchRule::dbus_interface() const {
    return m_priv->m_interface;
}

std::string MatchRule::member() const {
    return m_priv->m_member;
}

std::string MatchRule::match_rule() const {
    std::string match_rule = "type='" + m_priv->m_type + "'";

    if( !m_priv->m_interface.empty() ) { match_rule += ",interface='"   + m_priv->m_interface   + "'"; }

    if( !m_priv->m_member.empty() ) { match_rule += ",member='"      + m_priv->m_member      + "'"; }

    if( !m_priv->m_sender.empty() ) { match_rule += ",sender='"      + m_priv->m_sender      + "'"; }

    if( !m_priv->m_path.empty() ) { match_rule += ",path='"        + m_priv->m_path        + "'"; }

    if( !m_priv->m_destination.empty() ) { match_rule += ",destination='" + m_priv->m_destination + "'"; }

    return match_rule;
}

/***************************************************************************/
SignalMatchRule::SignalMatchRule( std::shared_ptr<MatchRuleData> data ) :
    MatchRule( "signal", data ){
}

MethodCallMatchRule::MethodCallMatchRule( std::shared_ptr<MatchRuleData> data ) :
    MatchRule( "method_call", data ){
}

MethodReturnMatchRule::MethodReturnMatchRule( std::shared_ptr<MatchRuleData> data ) :
    MatchRule( "method_return", data ){
}

ErrorMatchRule::ErrorMatchRule( std::shared_ptr<MatchRuleData> data ) :
    MatchRule( "error", data ){
}
