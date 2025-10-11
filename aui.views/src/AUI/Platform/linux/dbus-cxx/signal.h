// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2019 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <sstream>
#include <AUI/Platform/linux/dbus-cxx/utility.h>
#include <AUI/Platform/linux/dbus-cxx/signalbase.h>
#include <AUI/Platform/linux/dbus-cxx/headerlog.h>
#include <AUI/Platform/linux/dbus-cxx/signalmessage.h>

#ifndef DBUSCXX_DBUS_SIGNAL_H_
#define DBUSCXX_DBUS_SIGNAL_H_

namespace DBus {

template <typename... T_sig>
class Signal;

template <typename T_ret, typename... T_args>
class Signal<T_ret( T_args... )> {};

/**
 * Subclass of sigc::signal
 *
 * This class defines a subclass of the sigc::signal types and can be used
 * as its parent. The key difference is that when the signal is emitted
 * a DBus signal message is created using the parameters of the signal
 * emission and transmitted via the dbus in addition to any local
 * signal propagation that may take place.
 *
 * @ingroup signals
 *
 * @ingroup local
 *
 */
template <typename... T_type>
class Signal<void(T_type...)>
    : public sigc::signal<void( T_type... )>, public SignalBase {
private:
    Signal( const std::string& path, const std::string& interface_name, const std::string& member ):
        SignalBase( path, interface_name, member ) {
        m_internal_callback_connection =
            this->connect( sigc::mem_fun( *this, &Signal::internal_callback ) );
    }

public:
    static std::shared_ptr<Signal> create( const std::string& path, const std::string& interface_name, const std::string& member ) {
        return std::shared_ptr<Signal>( new Signal( path, interface_name, member ) );
    }

    //  virtual std::shared_ptr<signal_base> clone()
    //  {
    //    return std::shared_ptr<signal_base>( new signal(*this) );
    //  }

    /** Returns a DBus XML description of this interface */
    virtual std::string introspect( int space_depth = 0 ) const {
        std::ostringstream sout;
        std::string spaces;
        DBus::priv::dbus_function_traits<std::function<void( T_type... )>> method_sig_gen;

        for( int i = 0; i < space_depth; i++ ) { spaces += " "; }

        sout << spaces << "<signal name=\"" << name() << "\">\n";
        sout << spaces << method_sig_gen.introspect( m_arg_names, 0, spaces );
        sout << spaces << "</signal>\n";
        return sout.str();
    }

    virtual std::string arg_name( size_t i ) {
        if( i < m_arg_names.size() ) { return m_arg_names[i]; }

        return std::string();
    }

    virtual void set_arg_name( size_t i, const std::string& name ) {
        if( i < m_arg_names.size() ) {
            m_arg_names[i] = name;
        }
    }

protected:

    friend class Interface;

    std::vector<std::string> m_arg_names;

    sigc::connection m_internal_callback_connection;

    void internal_callback( T_type... args ) {
        std::shared_ptr<SignalMessage> __msg = SignalMessage::create( path(), interface_name(), name() );
        DBUSCXX_DEBUG_STDSTR( "DBus.Signal", "Sending following signal: "
            << __msg->path()
            << " "
            << __msg->interface_name()
            << " "
            << __msg->member() );

        if( !destination().empty() ) { __msg->set_destination( destination() ); }

        ( *__msg << ... << args );
        bool result = this->handle_dbus_outgoing( __msg );
        DBUSCXX_DEBUG_STDSTR( "DBus.Signal", "signal::internal_callback: result=" << result );
    }
};

} /* namespace DBus */

#endif /* _DBUS_CXX_SIGNAL_H_ */
