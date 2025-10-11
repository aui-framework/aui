// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *   Copyright (C) 2019 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <stdint.h>
#include <AUI/Platform/linux/dbus-cxx/signal.h>
#include <AUI/Platform/linux/dbus-cxx/signalproxy.h>
#include <AUI/Platform/linux/dbus-cxx/threaddispatcher.h>
#include <AUI/Platform/linux/dbus-cxx/errormessage.h>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "enums.h"
#include <future>
#include <queue>

#ifndef DBUSCXX_CONNECTION_H
#define DBUSCXX_CONNECTION_H

/** Same as DBUS_NAME_FLAG_ALLOW_REPLACEMENT.  See request_name(). */
#define DBUSCXX_NAME_FLAG_ALLOW_REPLACEMENT 0x01
/** Same as DBUS_NAME_FLAG_REPLACE_EXISTING.  See request_name(). */
#define DBUSCXX_NAME_FLAG_REPLACE_EXISTING 0x02
/** Same as DBUS_NAME_FLAG_DO_NOT_QUEUE.  See request_name(). */
#define DBUSCXX_NAME_FLAG_DO_NOT_QUEUE 0x04

#define DBUSCXX_INTERFACE_INTROSPECTABLE "org.freedesktop.DBus.Introspectable"
/** XML namespace of the introspection format version 1.0 */
#define DBUSCXX_INTROSPECT_1_0_XML_NAMESPACE         "http://www.freedesktop.org/standards/dbus"
/** XML public identifier of the introspection format version 1.0 */
#define DBUSCXX_INTROSPECT_1_0_XML_PUBLIC_IDENTIFIER "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
/** XML system identifier of the introspection format version 1.0 */
#define DBUSCXX_INTROSPECT_1_0_XML_SYSTEM_IDENTIFIER "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd"
/** XML document type declaration of the introspection format version 1.0 */
#define DBUSCXX_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE "<!DOCTYPE node PUBLIC \""\
    DBUSCXX_INTROSPECT_1_0_XML_PUBLIC_IDENTIFIER "\"\n\"" DBUSCXX_INTROSPECT_1_0_XML_SYSTEM_IDENTIFIER "\">\n"


namespace DBus {
class Message;
class Object;
class ObjectPathHandler;
class ObjectProxy;
class PendingCall;
class ReturnMessage;
class SignalMessage;
class Timeout;
class Watch;
class ThreadDispatcher;
class ErrorMessage;
class DBusDaemonProxy;

namespace priv {
class Transport;
}

/**
 * Connection point to the DBus
 *
 * @ingroup core
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class Connection : public std::enable_shared_from_this<Connection> {

private:
    Connection( BusType type );

    Connection( std::string address );

public:
    /**
     * Connects to a bus daemon.  The returned Connection will have authenticated
     * with the bus at this point, but it will not be registered.  bus_register()
     * must be called - the Dispatcher should do this automatically.
     *
     * @param is_private if true a private connection will be created. Otherwise a shared connection is created.
     */
    static std::shared_ptr<Connection> create( BusType type );

    /**
     * Create a new connection, connecting to the specified address.  This address
     * needs to be in the in DBus transport format (e.g. unix:path=/tmp/dbus-test).
     * The returned connection will have authenticated with the bus at this point,
     * but it will not be registered.  bus_register() must be called - the Dispatcher
     * should do this automatically.
     *
     * @param address
     * @return
     */
    static std::shared_ptr<Connection> create( std::string address );

    ~Connection();

    /** True if this is a valid connection; false otherwise */
    operator bool() const;

    /** True if this is a valid connection; false otherwise */
    bool is_valid() const;

    /** True if this connection is already registered */
    bool is_registered() const;

    /**
     * Registers this connection with the bus.  It is safe to call this
     * method multiple times.
     */
    bool bus_register();

    /** Gets the unique name of the connection as assigned by the message bus. */
    std::string unique_name() const;

    /**
     * Request the given name on the bus.
     *
     * @param name The name to request, e.g. "com.example.foo"
     * @param flags Any flags with the name; see DBUSCXX_NAME_FLAG_XXX macros
     * @return
     */
    RequestNameResponse request_name( const std::string& name, unsigned int flags = 0 );

    /**
     * Release the specified name, after requesting it via request_name
     *
     * @param name The name to release on the bus
     * @return
     */
    ReleaseNameResponse release_name( const std::string& name );

    /**
     * Check to see if the given name currently has an owner.
     *
     * Note that usage of this method in the form
     * if( !connection->name_has_owner( "foo.bar" ) ){
     *   connection->request_name( "foo.bar" );
     * }
     *
     * may be subject to race conditions, as multiple applications may be attempting
     * to request the bus name at the same time.  Instead, you should set the flags
     * in the request_name method to do this in a bus-atomic manner.
     *
     * @param name
     * @return
     */
    bool name_has_owner( const std::string& name ) const;

    /**
     * @brief start_service
     * @param name
     * @param flags
     * @return
     */
    StartReply start_service( const std::string& name, uint32_t flags = 0 ) const;

    bool add_match( const std::string& rule );

    void add_match_nonblocking( const std::string& rule );

    bool remove_match( const std::string& rule );

    bool is_connected() const;

    bool is_authenticated() const;

    bool is_anonymous() const;

    const char* server_id() const;

    /**
     * Queues up the message to be sent on the bus.
     *
     * @param message The message to send
     * @return The serial of the message
     */
    uint32_t send( const std::shared_ptr<const Message> message );

    /**
     * Blindly sends the message on the connection.  Since you don't get any kind of handle
     * back from this, you should really only use it for sending method returns and signals.
     */
    Connection& operator<<( std::shared_ptr<const Message> msg );

    /**
     * Send a CallMessage, and wait for the reply.
     *
     * If a timeout is processed, this will throw ErrorNoReply
     *
     * @param msg The message to send
     * @param timeout_milliseconds How long to wait for. If -1, a "sane default value" is used.
     * @return The return message
     */
    std::shared_ptr<ReturnMessage> send_with_reply_blocking( std::shared_ptr<const CallMessage> msg, int timeout_milliseconds = -1 );

    /**
     * A timeout-less version of @ref send_with_reply_blocking().
     */
    std::shared_ptr<ReturnMessage> send_with_reply_blocking_notimeout( std::shared_ptr<const CallMessage> msg );

    /**
     * Flushes all data out to the bus.  This should generally
     * be called from the dispatching thread, but it should be
     * able to be called from any thread.
     */
    void flush();

    DispatchStatus dispatch_status( ) const;

    /**
     * Dispatch the connection.  Dispatching involves writing messages
     * to the bus, reading messages from the bus, and acting on those
     * messages, if available.  This method can only be called from
     * the dispatching thread, and will throw an exception if it is
     * called from the wrong thread.
     *
     * This method, when called, will process at most one message.  This
     * is required so that responses to method calls will appear to be fully
     * blocking.
     *
     * @return The status of dispatching.  This method should be called
     * util the status is DispatchStatus::COMPLETE
     */
    DispatchStatus dispatch( );

    int unix_fd() const;

    int socket() const;

    bool has_messages_to_send();

    /**
     * This signal is emitted whenever we need to be dispatched.
     *
     * Cannot call dispatch() in a slot connected to this signal.
     *
     * Any slots that listen to this signal must be threadsafe, as this
     * may be emitted from any thread.
     */
    sigc::signal<void()>& signal_needs_dispatch();

    /**
     * Create and return a new object, registering the object automatically.  If the registering
     * fails, an invalid pointer will be returned.
     *
     * @param path The path of a new object, e.g. /opt/freedesktop/DBus
     * @param calling How to call this object(what thread).  Defaults to being the dispatching thread.
     * @return An invalid shared_ptr if registration was not successful, else a valid shared_ptr
     */
    std::shared_ptr<Object> create_object( const std::string& path,
                                           ThreadForCalling calling = ThreadForCalling::DispatcherThread );

    /**
     * Register an object with this connection.  The path that this object is accessible at
     * is part of the object.  You may also select which thread the object's methods should
     * be called on.
     *
     * @param object The object to export
     * @param calling The thread in which this object's methods will be called in.  Defaults to being the dispatching thread.
     * @return The status of registering an object to be exported.
     */
    RegistrationStatus register_object( std::shared_ptr<Object> object,
        ThreadForCalling calling = ThreadForCalling::DispatcherThread );

    /**
     * Change the thread that the methods on this object will be called from.  Note that this
     * object must already be registered with register_object.
     *
     * @param object
     * @param calling
     * @return
     */
    bool change_object_calling_thread( std::shared_ptr<Object> object,
                                       ThreadForCalling calling );

    /**
     * Change the thread that the signals on this ObjectProxy will be called from.
     * Note that this ObjectProxy must be related to this connection
     *
     * @param object The ObjectProxy that has the signals to listen for
     * @param calling
     * @return
     */
    bool change_object_proxy_calling_thread( std::shared_ptr<ObjectProxy> object,
                                             ThreadForCalling calling );

    std::shared_ptr<ObjectProxy> create_object_proxy( const std::string& path,
                                                      ThreadForCalling calling = ThreadForCalling::DispatcherThread );

    std::shared_ptr<ObjectProxy> create_object_proxy( const std::string& destination, const std::string& path,
                                                      ThreadForCalling calling = ThreadForCalling::DispatcherThread);

    /**
     * Add an ObjectProxy to this Connection.  Note that calling create_object_proxy() will call
     * this method automatically.
     *
     * @param obj The ObjectProxy to register with this connection
     * @return
     */
    bool register_object_proxy( std::shared_ptr<ObjectProxy> obj,
                                ThreadForCalling calling = ThreadForCalling::DispatcherThread );


    bool unregister_object_proxy( ObjectProxy* obj );

    bool unregister_object_proxy( std::shared_ptr<ObjectProxy> obj );

    /**
     * Unregister an object that has been added with register_object.
     *
     * @param path
     * @return
     */
    bool unregister_object( const std::string& path );

    /**
     * Create and return a signal proxy that lets you listen to signals sent on the DBus as a free proxy.
     * A free proxy is one that is not attached to an interface.
     *
     * Normally, you want to add signal proxies via Interface::add_signal.
     *
     * This method may be used when you need more control over the match rule for the signal
     * that you want to listen to.
     *
     * This method automatically calls add_signal_proxy.
     */
    template<typename... T_arg>
    std::shared_ptr<SignalProxy<T_arg...> > create_free_signal_proxy( const SignalMatchRule& rule,
                                                                 ThreadForCalling calling = ThreadForCalling::DispatcherThread ) {
        std::shared_ptr<SignalProxy<T_arg...> > sig;
        sig = SignalProxy<T_arg...>::create( rule );
        this->add_free_signal_proxy( sig, calling );
        return sig;
    }

    /**
     * Adds the given signal proxy to the connection.
     */
    std::shared_ptr<SignalProxyBase> add_free_signal_proxy( std::shared_ptr<SignalProxyBase> Signal,
                                                       ThreadForCalling calling = ThreadForCalling::DispatcherThread );

    /**
     * Remove a free signal proxy, so that it will not be called anymore.
     *
     * @param proxy
     * @return
     */
    bool remove_free_signal_proxy( std::shared_ptr<SignalProxyBase> proxy );

    /** Gets all the free signal handlers */
    const std::vector<std::shared_ptr<SignalProxyBase>> get_free_signal_proxies();

    /** Gets the signal handlers for a specific interface */
    std::vector<std::shared_ptr<SignalProxyBase>> get_free_signal_proxies( const std::string& interface_name );

    /** Gets the signal handlers for a specific interface and member */
    std::vector<std::shared_ptr<SignalProxyBase>> get_free_signal_proxies( const std::string& interface_name, const std::string& member );

    /**
     * Create a free signal, that when it is emitted will send that signal over the DBus.
     * A free signal is a signal that is not associated with an Object.
     */
    template <class T_arg>
    std::shared_ptr<Signal<T_arg> > create_free_signal( const std::string& path, const std::string& interface_name, const std::string& member ) {
        std::shared_ptr<Signal<T_arg> > sig;
        sig = Signal<T_arg>::create( path, interface_name, member );
        sig->set_connection( shared_from_this() );
        return sig;
    }

    std::string introspect( const std::string& destination, const std::string& path );

    /**
     * Set the ID of the thread that all of the dispatching hapens from.
     * If a blocking call is attempted on the non-dispatching thread,
     * that thread will block until the dispatching thread handles the data.
     *
     * By default, the dispatching thread is the thread that this Connection
     * was created in.  If using the default Dispatcher, this will be set automatically.
     *
     * @param tid
     */
    void set_dispatching_thread( std::thread::id tid );

    /**
     * Add a thread dispatcher that will handle messages for a given thread.
     * This method must be called from the thread that this ThreadDispatcher
     * is handling messages for.
     *
     * This is a weak_ptr, as the intention is to simply declare a local
     * ThreadDispatcher object inside of the thread.  When the thread exits,
     * the ThreadDispatcher will be deconstructed.  The next time a message
     * for this thread comes in, anything that is associated with this thread
     * will become unexported.
     *
     * @param disp The thread dispatcher to associate with this connection.
     */
    void add_thread_dispatcher( std::weak_ptr<ThreadDispatcher> disp );

private:
    /**
     * Depending on what thread this is called from,
     * will either notify the dispatcher that we need to be
     * dispatched, or will do the dispatching needed.
     */
    void notify_dispatcher_or_dispatch();

    /**
     * Write a single message, return the serial of this message.
     * This should me called with a lock on m_outgoingLock
     *
     * @param msg
     * @return
     */
    uint32_t write_single_message( std::shared_ptr<const Message> msg );

    void process_single_message();

    void remove_invalid_threaddispatchers_and_associated_objects();

    std::shared_ptr<ReturnMessage> send_with_reply_blocking_impl( std::shared_ptr<const CallMessage> msg, int timeout_milliseconds, bool disable_timeout );

    /**
     * Send an error back to the calling application based on HandlerResult.  No-op if the
     * result indicates that there is no error.
     *
     */
    void send_error_on_handler_result( std::shared_ptr<const CallMessage> msg, HandlerResult result );

    void process_call_message( std::shared_ptr<const CallMessage> msg );
    void process_signal_message( std::shared_ptr<const SignalMessage> msg );

    std::thread::id thread_id_from_calling( ThreadForCalling calling );

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;
};

inline
std::shared_ptr<DBus::Connection> operator<<( std::shared_ptr<DBus::Connection> ptr, std::shared_ptr<DBus::Message> msg ) {
    if( !ptr ) { return ptr; }

    *ptr << msg;
    return ptr;
}

inline
std::shared_ptr<DBus::Connection> operator<<( std::shared_ptr<DBus::Connection> ptr, std::shared_ptr<const DBus::Message> msg ) {
    if( !ptr ) { return ptr; }

    *ptr << msg;
    return ptr;
}

inline
std::shared_ptr<DBus::Connection> operator<<( std::shared_ptr<DBus::Connection> ptr, std::shared_ptr<DBus::ReturnMessage> msg ) {
    if( !ptr ) { return ptr; }

    *ptr << msg;
    return ptr;
}

inline
std::shared_ptr<DBus::Connection> operator<<( std::shared_ptr<DBus::Connection> ptr, std::shared_ptr<const DBus::ReturnMessage> msg ) {
    if( !ptr ) { return ptr; }

    *ptr << msg;
    return ptr;
}

inline
std::shared_ptr<DBus::Connection> operator<<( std::shared_ptr<DBus::Connection> ptr, std::shared_ptr<DBus::SignalMessage> msg ) {
    if( !ptr ) { return ptr; }

    *ptr << msg;
    return ptr;
}

inline
std::shared_ptr<DBus::Connection> operator<<( std::shared_ptr<DBus::Connection> ptr, std::shared_ptr<const DBus::SignalMessage> msg ) {
    if( !ptr ) { return ptr; }

    *ptr << msg;
    return ptr;
}

inline
std::shared_ptr<DBus::Connection> operator<<( std::shared_ptr<DBus::Connection> ptr, std::shared_ptr<DBus::ErrorMessage> msg ) {
    if( !ptr ) { return ptr; }

    *ptr << msg;
    return ptr;
}

inline
std::shared_ptr<DBus::Connection> operator<<( std::shared_ptr<DBus::Connection> ptr, std::shared_ptr<const DBus::ErrorMessage> msg ) {
    if( !ptr ) { return ptr; }

    *ptr << msg;
    return ptr;
}

} /* namespace DBus */

#endif
