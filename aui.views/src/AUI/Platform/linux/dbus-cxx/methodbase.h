// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *   Copyright (C) 2019-2020 by Robert Middleton                           *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <exception>
#include <stdint.h>
#include <AUI/Platform/linux/dbus-cxx/callmessage.h>
#include <AUI/Platform/linux/dbus-cxx/errormessage.h>
#include <AUI/Platform/linux/dbus-cxx/headerlog.h>
#include <AUI/Platform/linux/dbus-cxx/utility.h>
#include <stddef.h>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include "enums.h"
#include "error.h"

#ifndef DBUSCXX_METHODBASE_H
#define DBUSCXX_METHODBASE_H

namespace DBus {

class Connection;
class Message;
class ReturnMessage;

/**
 * Base class for all methods(proxy and local).
 *
 * Note that the name of the method is immutable and cannot be changed once a method is created.
 *
 * @ingroup local
 * @ingroup objects
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class MethodBase {
protected:

    MethodBase( const std::string& name );

protected:
    uint32_t sendMessage( std::shared_ptr<Connection> connection, const std::shared_ptr<const Message> );

public:
    virtual ~MethodBase();

    const std::string& name() const;

    virtual HandlerResult handle_call_message( std::shared_ptr<Connection> connection, std::shared_ptr<const CallMessage> message ) = 0;

    /** Returns a DBus XML description of this interface */
    virtual std::string introspect( int = 0 ) const { return std::string(); };

    std::string arg_name( size_t i ) const;

    void set_arg_name( size_t i, const std::string& name );

    const std::vector<std::string>& arg_names() const;

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;
};

template <typename T_type>
class Method : public MethodBase {
private:
    Method( const std::string& name ) : MethodBase( name ) {}

public:
    static std::shared_ptr<Method<T_type>> create( const std::string& name ) {
        return std::shared_ptr<Method<T_type>>( new Method<T_type>( name ) );
    }

    void set_method( sigc::slot<T_type> slot ) { m_slot = slot; }

    virtual std::string introspect( int space_depth = 0 ) const {
        std::ostringstream sout;
        std::string spaces;
        DBus::priv::dbus_function_traits<std::function<T_type>> method_sig_gen;

        for( int i = 0; i < space_depth; i++ ) { spaces += " "; }

        sout << spaces << "<method name=\"" << name() << "\">\n";
        sout << method_sig_gen.introspect( arg_names(), 0, spaces + "  " );
        sout << spaces << "</method>\n";
        return sout.str();
    }

    virtual HandlerResult handle_call_message( std::shared_ptr<Connection> connection, std::shared_ptr<const CallMessage> message ) {
        std::ostringstream debug_str;
        DBus::priv::dbus_function_traits<std::function<T_type>> method_sig_gen;

        debug_str << "DBus::Method<";
        debug_str << method_sig_gen.debug_string();
        debug_str << ">::handle_call_message method=";
        debug_str << name();
        DBUSCXX_DEBUG_STDSTR( "DBus.Method", debug_str.str() );

        if( !connection || !message ) { return HandlerResult::Not_Handled; }

        try {
            std::shared_ptr<ReturnMessage> retmsg = message->create_reply();

            if( !retmsg ) { return HandlerResult::Not_Handled; }

            method_sig_gen.extractAndCall( message, retmsg, m_slot );

            sendMessage( connection, retmsg );
        } catch( ErrorInvalidTypecast& e ) {
            std::shared_ptr<ErrorMessage> errmsg = message->create_error_reply();

            if( !errmsg ) { return HandlerResult::Not_Handled; }

            errmsg->set_name(DBUSCXX_ERROR_INVALID_SIGNATURE);
            if( e.what() ){
                errmsg << e.what();
            }

            sendMessage( connection, errmsg );
        } catch( const std::exception& e ) {
            std::shared_ptr<ErrorMessage> errmsg = message->create_error_reply();

            if( !errmsg ) { return HandlerResult::Not_Handled; }

            errmsg->set_name(DBUSCXX_ERROR_FAILED);
            if( e.what() ){
                errmsg << e.what();
            }

            sendMessage( connection, errmsg );
        } catch( ... ) {
            std::ostringstream stream;
            stream << "DBus-cxx " << DBUS_CXX_PACKAGE_MAJOR_VERSION << "."
                << DBUS_CXX_PACKAGE_MINOR_VERSION << "."
                << DBUS_CXX_PACKAGE_MICRO_VERSION
                << ": unknown error(uncaught exception)";
            std::shared_ptr<ErrorMessage> errmsg = message->create_error_reply();

            if( !errmsg ) { return HandlerResult::Not_Handled; }

            errmsg->set_name(DBUSCXX_ERROR_FAILED);
            errmsg << stream.str();

            sendMessage( connection, errmsg );
        }

        return HandlerResult::Handled;
    }


private:
    sigc::slot<T_type> m_slot;
};

}

#endif
