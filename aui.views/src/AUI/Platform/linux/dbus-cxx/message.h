// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2009,2010 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <stdint.h>
#include <AUI/Platform/linux/dbus-cxx/error.h>
#include <AUI/Platform/linux/dbus-cxx/messageappenditerator.h>
#include <AUI/Platform/linux/dbus-cxx/messageiterator.h>
#include <memory>
#include <string>
#include "enums.h"

#include <AUI/Platform/linux/dbus-cxx/variant.h>

#ifndef DBUSCXX_MESSAGE_H
#define DBUSCXX_MESSAGE_H

#define DBUSCXX_MESSAGE_NO_REPLY_EXPECTED               0x01
#define DBUSCXX_MESSAGE_NO_AUTO_START_FLAG              0x02
// This is set in CallMessage only.
#define DBUSCXX_MESSAGE_ALLOW_INTERACTIVE_AUTHORIZATION 0x04

namespace DBus {
class ReturnMessage;

/**
 * @defgroup message DBus Messages
 * Messages may be either sent across the DBus or received from the DBus
 */

/**
 * This class represents a basic DBus message and also serves as a base
 * class for the specialized message types (call, return, signal, error).
 *
 * This class cannot be initialized on its own; create a new Message from
 * one of the subclasses
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 *
 * @ingroup message
 */
class Message {
protected:

    Message();

public:

    virtual ~Message();

    bool operator == ( const Message& other );

    bool is_valid() const;

    void invalidate();

    operator bool() const;

    uint32_t serial() const;

    virtual MessageType type() const = 0;

    void set_auto_start( bool auto_start );

    /**
     * Returns true if the bus is allowed to start an owner for this
     * message's destination if it is not running
     *
     * @return
     */
    bool auto_start();

    /**
     * Set the destination of this message
     *
     * @param s
     * @return true if the destination was able to be set, false if the
     * destination was unable to be validated
     */
    bool set_destination( const std::string& s );

    std::string destination() const;

    std::string sender() const;

    Signature signature() const;

    template <typename T>
    MessageIterator operator>>( T& value ) const {
        MessageIterator iter = this->begin();
        iter >> value;
        return iter;
    }

    template <typename T>
    MessageAppendIterator operator<<( const T& value ) {
        MessageAppendIterator aiter( *this );
        aiter << value;
        return aiter;
    }

    MessageIterator begin() const;

    MessageIterator end() const;

    MessageAppendIterator append();

    /**
     * Serialize this message to the given vector.  The vector will be resized
     * but not cleared.  Can fail with an error under the following circumstances:
     * - Invalid message type passed in
     * - Method return or method call does not have serial set
     *
     * @param vec The location to serialize the message to.
     * @param serial The serial of the message.
     * @return True if the message was able to be serialized, false otherwise.
     */
    bool serialize_to_vector( std::vector<uint8_t>* vec, uint32_t serial ) const;

    /**
     * Returns the given header field(if it exists), otherwise returns a default
     * constructed variant.
     *
     * @param field The field number to get
     * @return The data, otherwise a default-constructed variant.
     */
    Variant header_field( MessageHeaderFields field ) const;

    /**
     * The message flags, as the marshaled byte
     * @return
     */
    uint8_t flags() const;

    /**
     * Set the given header field.  Returns the previously set value, if it exists.
     *
     * @param field The field to set
     * @param value The value to set the header field to
     * @return The old value, if it exists.
     */
    Variant set_header_field( MessageHeaderFields field, Variant value );

    Endianess endianess() const;

    const std::vector<int>& filedescriptors() const;

    static std::shared_ptr<Message> create_from_data( uint8_t* data, uint32_t data_len, std::vector<int> fds = std::vector<int>() );

protected:

    void append_signature( std::string toappend );

    /**
     * Clears the signature and the data, so you can re-append data
     */
    void clear_sig_and_data();

    void set_flags( uint8_t flags );

private:
    std::vector<uint8_t>* body();
    const std::vector<uint8_t>* body() const;
    void add_filedescriptor( int fd );
    uint32_t filedescriptors_size() const;
    int filedescriptor_at_location( int location ) const;

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;

    friend class MessageAppendIterator;
    friend class MessageIterator;
    friend std::ostream& operator<<( std::ostream& os, const DBus::Message* msg );

};


template <typename T>
inline
DBus::MessageIterator operator>>( std::shared_ptr<const DBus::Message> ptr, T& value ) {
    if( !ptr ) { throw DBus::ErrorInvalidSharedPtr(); }

    return ( *ptr ) >> value;
}

template <typename T>
inline
DBus::MessageAppendIterator operator<<( std::shared_ptr<DBus::Message> ptr, const T& value ) {
    if( !ptr ) { throw DBus::ErrorInvalidSharedPtr(); }

    return ( *ptr ) << value;
}

}

#endif
