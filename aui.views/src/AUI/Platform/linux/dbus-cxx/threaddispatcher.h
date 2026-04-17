// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_THREADDISPATCHER_H
#define DBUSCXX_THREADDISPATCHER_H

#include <memory>

namespace DBus {

class Message;
class Object;
class SignalProxyBase;
class SignalMessage;
class CallMessage;

/**
 * A ThreadDispatcher is responsible for executing method calls
 * and signals on objects in the given thread.  If you are using
 * the standard DBus::Dispatcher class, you can either run all method calls
 * in the dispatcher thread, and create a ThreadDispatcher in the main
 * thread with a main loop.
 *
 * If you're using Qt, link with dbus-cxx-qt and use the QtThreadDispatcher
 * class.
 *
 * If you're using GLib, link with dbus-cxx-glib and use the GLibThreadDispatcher
 * class.
 *
 * Note that all methods in this class are called from the dispatcher thread.
 */
class ThreadDispatcher {
public:
    virtual ~ThreadDispatcher();

    /**
     * When a new message comes in that needs to be processed, this method
     * is called with the object that the method needs to go to, and the
     * message to send to this object.
     *
     * Generally, this method should push the object/message onto some sort of
     * mutex-locked queue and then wakeup this thread.  The thread will
     * then lock the mutex, pop an element off of the queue, and direct
     * the message to the given object via the handle_message method
     *
     * @param object The object to send the message to
     * @param message The message to send to the object
     */
    virtual void add_message( std::shared_ptr<Object> object, std::shared_ptr<const CallMessage> message ) = 0;

    /**
     * Add a signal proxy that must emit its signals from the thread represented
     * by this ThreadDispatcher
     *
     * @param handler A handler that can handle incoming signals
     */
    virtual void add_signal_proxy( std::shared_ptr<SignalProxyBase> handler ) = 0;

    /**
     * Remove a signal proxy.  Note that it is possible that this method is called
     * with a signal_proxy_base that has not been previously added with add_signal_proxy;
     * this is not an error and should not cause unintended consequences.
     *
     * @param handler The handler to remove
     * @returns True if the signal proxy was removed, false otherwise.
     */
    virtual bool remove_signal_proxy( std::shared_ptr<SignalProxyBase> handler ) = 0;

    /**
     * When a new signal message comes in that needs to be processed, this method
     * is called with the SignalMessage that must be emitted from this thread.
     *
     * Generally, this method should push the SignalMesage onto some sort of
     * mutex-locked queue and then wakeup this thread.  This thread will then
     * lock the mutex, pop an element off of the queue, and loop through all
     * of the signal proxys (added via add_signal_proxy) and calling the
     * SignalProxyBase::handle_signal method.
     *
     * @param message The message to be emitted
     */
    virtual void add_signal( std::shared_ptr<const SignalMessage> message ) = 0;

};

} /* namespace DBus */

#endif /* DBUSCXX_THREADDISPATCHER_H */
