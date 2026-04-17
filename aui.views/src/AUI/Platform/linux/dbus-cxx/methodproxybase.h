// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *   Copyright (C) 2019 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/callmessage.h>
#include <AUI/Platform/linux/dbus-cxx/headerlog.h>
#include <AUI/Platform/linux/dbus-cxx/utility.h>
#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include "messageappenditerator.h"
#include <future>

#ifndef DBUSCXX_METHODPROXYBASE_H
#define DBUSCXX_METHODPROXYBASE_H

namespace DBus {

class InterfaceProxy;
class PendingCall;
class ReturnMessage;
template <typename signature> class MethodProxy;

/**
 * Base class for all proxy(remote) methods.  This class allows remote methods
 * to be used in a generic manner, as the MethodProxy class is templated and
 * thus cannot be used in a generic manner(e.g. stored in a vector).
 *
 * @ingroup objects
 * @ingroup proxy
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class MethodProxyBase {
protected:

    MethodProxyBase( const std::string& name );

    MethodProxyBase( const MethodProxyBase& other );

public:
    static std::shared_ptr<MethodProxyBase> create( const std::string& name );

    ~MethodProxyBase();

    InterfaceProxy* interface_name() const;

    const std::string& name() const;

    std::shared_ptr<CallMessage> create_call_message( ) const;

    std::shared_ptr<const ReturnMessage> call( std::shared_ptr<const CallMessage>, int timeout_milliseconds = -1 ) const;

    /**
     * Enable interactive authorization for method call.
     *
     * To quote the [D-Bus
     * specification](https://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-messages):
     *
     * > This flag may be set on a method call message to inform the receiving side that
     * > the caller is prepared to wait for interactive authorization, which might take
     * > a considerable time to complete. For instance, if this flag is set, it would be
     * > appropriate to query the user for passwords or confirmation via Polkit or a
     * > similar framework.
     * >
     * > This flag is only useful when unprivileged code calls a more privileged method
     * > call, and an authorization framework is deployed that allows possibly interactive
     * > authorization. If no such framework is deployed it has no effect. This flag should
     * > not be set by default by client implementations. If it is set, the caller should
     * > also set a suitably long timeout on the method call to make sure the user
     * > interaction may complete. This flag is only valid for method call messages, and
     * > shall be ignored otherwise.
     * >
     * > If a this flag is not set on a method call, and a service determines that the
     * > requested operation is not allowed without interactive authorization, but could
     * > be allowed after successful interactive authorization, it may return the
     * > @ref DBus::ErrorInteractiveAuthorizationRequired error.
     * >
     * > The absence of this flag does not guarantee that interactive authorization will
     * > not be applied, since existing services that pre-date this flag might already use
     * > interactive authorization. However, existing D-Bus APIs that will use interactive
     * > authorization should document that the call may take longer than usual, and new
     * > D-Bus APIs should avoid interactive authorization in the absence of this flag.
     *
     * Disabled by default.
     *
     * @note
     * Any call messages created by @ref create_call_message() before this function is
     * called will retain the old behavior. If that is not desirable, call @ref
     * create_call_message() again after this function is called and use the new return
     * value.
     *
     * @param timeout Timeout of the request in milliseconds. If set to zero, no timeout
     *                is imposed. This timeout overrides the `timeout_milliseconds` argument
     *                of @ref call() (used in @ref MethodProxy).
     *
     * @throws std::invalid_argument If `timeout_milliseconds` doesn't fit into a `signed int`.
     */
    void enable_interactive_authorization( unsigned int timeout_milliseconds = 0 );

    /**
     * Disable interactive authorization for method call.
     *
     * See @ref enable_interactive_authorization() for more info.
     *
     * If @ref enable_interactive_authorization() was called already, the timeout set by it
     * is cleared by this function. The `timeout_milliseconds` argument of @ref call() will
     * no longer be overriden.
     */
    void disable_interactive_authorization();

    //      std::shared_ptr<PendingCall> call_async( std::shared_ptr<const CallMessage>, int timeout_milliseconds=-1 ) const;

private:
    void set_interface( InterfaceProxy* proxy );

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;

    // Declare InterfaceProxy as a friend so that it can set the interface
    friend class InterfaceProxy;
};


/**
 * MethodProxy specialization for void return type.
 *
 * @param T_arg Method arguments
 */
template<typename... T_arg>
class MethodProxy<void( T_arg... )> : public MethodProxyBase {
protected:
    MethodProxy( const std::string& name ) :
        MethodProxyBase( name ) {}

public:
    void operator()( T_arg... args ) {
        std::ostringstream debug_str;
        DBus::priv::dbus_function_traits<std::function<void( T_arg... )>> method_sig_gen;

        debug_str << "DBus::MethodProxy<";
        debug_str << method_sig_gen.debug_string();
        debug_str << "> calling method=";
        debug_str << name();
        DBUSCXX_DEBUG_STDSTR( "DBus.MethodProxy", debug_str.str() );

        std::shared_ptr<CallMessage> _callmsg = this->create_call_message();
        ( *_callmsg << ... << args );
        std::shared_ptr<const ReturnMessage> retmsg = this->call( _callmsg, -1 );
    }

    std::future<void> call_async( T_arg... args ) {
        std::ostringstream debug_str;
        DBus::priv::dbus_function_traits<std::function<void( T_arg... )>> method_sig_gen;

        debug_str << "DBus::MethodProxy<";
        debug_str << method_sig_gen.debug_string();
        debug_str << "> calling async method=";
        debug_str << name();
        DBUSCXX_DEBUG_STDSTR( "DBus.MethodProxy", debug_str.str() );

        return std::async( std::launch::async, *this, args... );
    }

    static std::shared_ptr<MethodProxy> create( const std::string& name ) {
        return std::shared_ptr<MethodProxy>( new MethodProxy( name ) );
    }
};

/**
 * MethodProxy specialization for non-void return type.
 *
 * @param T_return Return type(not void)
 * @param T_arg Method arguments
 */
template<typename T_return, typename... T_arg>
class MethodProxy<T_return( T_arg... )> : public MethodProxyBase {
protected:
    MethodProxy( const std::string& name ) :
        MethodProxyBase( name ) {}

public:
    T_return operator()( T_arg... args ) {
        std::ostringstream debug_str;
        DBus::priv::dbus_function_traits<std::function<T_return( T_arg... )>> method_sig_gen;

        debug_str << "DBus::MethodProxy<";
        debug_str << method_sig_gen.debug_string();
        debug_str << "> calling method=";
        debug_str << name();
        DBUSCXX_DEBUG_STDSTR( "DBus.MethodProxy", debug_str.str() );

        std::shared_ptr<CallMessage> _callmsg = this->create_call_message();
        MessageAppendIterator iter = _callmsg->append();
        ( void )( iter << ... << args );
        std::shared_ptr<const ReturnMessage> retmsg = this->call( _callmsg, -1 );
        T_return _retval;
        retmsg >> _retval;
        return _retval;
    }

    std::future<T_return> call_async( T_arg... args ) {
        std::ostringstream debug_str;
        DBus::priv::dbus_function_traits<std::function<void( T_arg... )>> method_sig_gen;

        debug_str << "DBus::MethodProxy<";
        debug_str << method_sig_gen.debug_string();
        debug_str << "> calling async method=";
        debug_str << name();
        DBUSCXX_DEBUG_STDSTR( "DBus.MethodProxy", debug_str.str() );

        return std::async( std::launch::async, *this, args... );
    }

    static std::shared_ptr<MethodProxy> create( const std::string& name ) {
        return std::shared_ptr<MethodProxy>( new MethodProxy( name ) );
    }
};

}

#endif
