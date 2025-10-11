// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/signalbase.h>
#include <AUI/Platform/linux/dbus-cxx/signalmessage.h>
#include <AUI/Platform/linux/dbus-cxx/utility.h>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include "enums.h"
#include "error.h"
#include "headerlog.h"
#include "messageiterator.h"
#include "matchrule.h"

#ifndef DBUSCXX_SIGNALPROXYBASE_H
#define DBUSCXX_SIGNALPROXYBASE_H

namespace DBus {
template <typename type> class SignalProxy;

/**
 * Base class for a signal proxy that allows you to listen for signals.  This is required
 * to use signals in a generic manner, as the subclasses are all templated.
 *
 * @ingroup proxy
 * @ingroup signals
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class SignalProxyBase: public SignalBase {
public:
    virtual HandlerResult handle_signal( std::shared_ptr<const SignalMessage> );

    const std::string& match_rule() const;

    void update_match_rule();

protected:
    SignalProxyBase( const SignalMatchRule& matchRule );

    virtual ~SignalProxyBase();

    bool matches( std::shared_ptr<const SignalMessage> msg );

    /**
     * This method is needed to be able to create a duplicate of a child
     * capable of parsing their specific template type message.
     */
    //      virtual std::shared_ptr<signal_base> clone() = 0;

    virtual HandlerResult on_dbus_incoming( std::shared_ptr<const SignalMessage> msg ) = 0;

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;
};

/**
 * Subclass of the sigc::signal templates.
 * A signal_proxy allows you to listen for signals that are emitted by other applications on the DBus.
 * Note that because DBus signals never have a return type, neither can the handling function.
 *
 * @ingroup signals
 * @ingroup proxy
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
template <typename... T_arg>
class SignalProxy<void(T_arg...)>
    : public sigc::signal<void(T_arg...)>, public SignalProxyBase {
public:
    SignalProxy( const SignalMatchRule& matchRule ):
        SignalProxyBase( matchRule )
    {  }

    static std::shared_ptr<SignalProxy> create( const SignalMatchRule& matchRule )
    { return std::shared_ptr<SignalProxy>( new SignalProxy( matchRule ) ); }

protected:
    HandlerResult on_dbus_incoming( std::shared_ptr<const SignalMessage> msg ) {
        std::tuple<T_arg...> tup_args;
        std::ostringstream debug_str;
        DBus::priv::dbus_function_traits<std::function<void( T_arg... )>> method_sig_gen;

        debug_str << "DBus::signal_proxy<";
        debug_str << method_sig_gen.debug_string();
        debug_str << ">::on_dbus_incoming method=";
        debug_str << msg->member();
        DBUSCXX_DEBUG_STDSTR( "DBus.signal_proxy", debug_str.str() );

        try {
            MessageIterator i = msg->begin();
            std::apply( [i]( auto&& ...arg ) mutable {
                ( void )( i >> ... >> arg );
            },
            tup_args );
            std::apply( &SignalProxy::emit, std::tuple_cat( std::make_tuple( this ), tup_args ) );
        } catch( ErrorInvalidTypecast& e ) {
            DBUSCXX_DEBUG_STDSTR( "DBus.signal_proxy", "Caught error invalid typecast" );
            return HandlerResult::Not_Handled;
        } catch( ... ) {
            DBUSCXX_DEBUG_STDSTR( "DBus.signal_proxy", "Unknown exception" );
            return HandlerResult::Not_Handled;
        }

        return HandlerResult::Handled;
    }

};


}

#endif
