// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_MATCH_RULE_H
#define DBUSCXX_MATCH_RULE_H

#include <string>
#include <memory>

namespace DBus {

class MatchRuleBuilder;
class MatchRuleData;

/**
 * Immutable class that represents a match rule for DBus.
 */
class MatchRule {
protected:
    MatchRule( std::string type, const std::shared_ptr<MatchRuleData> );

public:
    std::string match_rule() const;

    std::string path() const;

    std::string dbus_interface() const;

    std::string member() const;

private:
    std::shared_ptr<MatchRuleData> m_priv;

    friend class MatchRuleBuilder;
};

/**
 * A special MatchRule for signals.
 */
class SignalMatchRule : public MatchRule {
private:
    SignalMatchRule( std::shared_ptr<MatchRuleData> );

    friend class MatchRuleBuilder;
};

class MethodCallMatchRule : public MatchRule {
private:
    MethodCallMatchRule( std::shared_ptr<MatchRuleData> );

    friend class MatchRuleBuilder;
};

class MethodReturnMatchRule : public MatchRule {
private:
    MethodReturnMatchRule( std::shared_ptr<MatchRuleData> );

    friend class MatchRuleBuilder;
};

class ErrorMatchRule : public MatchRule {
private:
    ErrorMatchRule( std::shared_ptr<MatchRuleData> );

    friend class MatchRuleBuilder;
};

/**
 * A builder to create match rules.  When you are done building the match rule,
 * call the appropriate method(asSignalMatch, asMethodCallMatch, etc).
 */
class MatchRuleBuilder {
protected:
    MatchRuleBuilder();

public:
    MatchRuleBuilder& set_path( const std::string& path );

    MatchRuleBuilder& set_interface( const std::string& interface_name );

    MatchRuleBuilder& set_member( const std::string& member );

    MatchRuleBuilder& set_sender( const std::string& sender );

    MatchRuleBuilder& set_destination( const std::string& destination );

    SignalMatchRule as_signal_match();

    MethodCallMatchRule as_method_call_match();

    MethodReturnMatchRule as_method_return_match();

    ErrorMatchRule as_error_match();

    static MatchRuleBuilder create();

private:
    std::shared_ptr<MatchRuleData> m_priv;
};



} /* namespace DBus */

#endif /* DBUSCXX_MATCH_RULE_H */
