// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2009 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "message.h"
#include "messageappenditerator.h"
#include "messageiterator.h"
#include "returnmessage.h"
#include "callmessage.h"
#include "errormessage.h"
#include "signalmessage.h"
#include "variant.h"
#include "marshaling.h"
#include "demarshaling.h"
#include <AUI/Platform/linux/dbus-cxx/dbus-cxx-private.h>
#include <AUI/Platform/linux/dbus-cxx/simplelogger.h>
#include "validator.h"
#include "utility.h"

#include <unistd.h>

static const char* LOGGER_NAME = "DBus.Message";

namespace DBus {

class Message::priv_data {
public:
    priv_data() :
        m_valid( true ),
        m_endianess( DBus::default_endianess() ),
        m_flags( 0 ),
        m_serial( 0 )
    {}


    bool m_valid;
    std::map<MessageHeaderFields, Variant> m_headerMap;
    std::vector<uint8_t> m_body;
    Endianess m_endianess;
    uint8_t m_flags;
    std::vector<int> m_filedescriptors;
    uint32_t m_serial;
};

Message::Message() {
    m_priv = std::make_unique<priv_data>();
}

Message::~Message() {
    for( int i : m_priv->m_filedescriptors ) {
        close( i );
    }
}


bool Message::operator == ( const Message& other ) {
    // First, really simple: let's just check the type
    if( other.type() != type() ) {
        return false;
    }

    // Next, let's check the headers, since those will likely not be the same if the messages are different
    bool headersEqual =
        header_field( MessageHeaderFields::Path ) == other.header_field( MessageHeaderFields::Path ) &&
        header_field( MessageHeaderFields::Member ) == other.header_field( MessageHeaderFields::Member ) &&
        header_field( MessageHeaderFields::Sender ) == other.header_field( MessageHeaderFields::Sender ) &&
        header_field( MessageHeaderFields::Unix_FDs ) == other.header_field( MessageHeaderFields::Unix_FDs ) &&
        header_field( MessageHeaderFields::Interface ) == other.header_field( MessageHeaderFields::Interface ) &&
        header_field( MessageHeaderFields::Signature ) == other.header_field( MessageHeaderFields::Signature ) &&
        header_field( MessageHeaderFields::Error_Name ) == other.header_field( MessageHeaderFields::Error_Name ) &&
        header_field( MessageHeaderFields::Destination ) == other.header_field( MessageHeaderFields::Destination ) &&
        header_field( MessageHeaderFields::Reply_Serial ) == other.header_field( MessageHeaderFields::Reply_Serial );

    bool dataEqual = false;

    if( headersEqual ) {
        // Okay, all of the headers are equal at this point, now we can check the raw data
        dataEqual = m_priv->m_body == other.m_priv->m_body;
    }

    return  headersEqual && dataEqual;
}

bool Message::is_valid() const {
    // TODO fix this
    //     return ( m_cobj != nullptr and m_valid );
    return m_priv->m_valid;
}

void Message::invalidate() {
    m_priv->m_valid = false;
}

Message::operator bool() const {
    return this->is_valid();
}

uint32_t Message::serial() const {
    return m_priv->m_serial;
}

//   Message Message::copy()
//   {
// TODO fix
//     return Message( *this, CreateMethod::COPY );
//   }

void Message::set_auto_start( bool auto_start ) {
    if( auto_start ) {
        m_priv->m_flags &= ~DBUSCXX_MESSAGE_NO_AUTO_START_FLAG;
    } else {
        m_priv->m_flags |= DBUSCXX_MESSAGE_NO_AUTO_START_FLAG;
    }
}

bool Message::auto_start() {
    return m_priv->m_flags & DBUSCXX_MESSAGE_NO_AUTO_START_FLAG;
}

bool Message::set_destination( const std::string& s ) {
    if( Validator::validate_bus_name( s ) == false ) { return false; }

    m_priv->m_headerMap[ MessageHeaderFields::Destination ] = DBus::Variant( s );
    return true;
}

std::string Message::destination() const {
    Variant destination = header_field( MessageHeaderFields::Destination );

    if( destination.type() == DataType::STRING ) {
        return destination.to_string();
    }

    return "";
}

std::string Message::sender() const {
    Variant sender = header_field( MessageHeaderFields::Sender );

    if( sender.type() == DataType::STRING ) {
        return sender.to_string();
    }

    return "";
}

MessageIterator Message::begin() const {
    return MessageIterator( *this );
}

MessageIterator Message::end() const {
    return MessageIterator();
}

MessageAppendIterator Message::append() {
    return MessageAppendIterator( *this );
}

Signature Message::signature() const {
    Variant v = header_field( MessageHeaderFields::Signature );

    if( v.type() == DataType::SIGNATURE ) {
        return v.to_signature();
    }

    return Signature();
}

bool Message::serialize_to_vector( std::vector<uint8_t>* vec, uint32_t serial ) const {
    Marshaling marshal( vec, DBus::default_endianess() );
    Variant serialHeader = header_field( MessageHeaderFields::Reply_Serial );
    bool mustHaveSerial = false;

    vec->reserve( vec->size() + m_priv->m_body.size() + 256 );
    if(DBus::default_endianess() == Endianess::Little){
        marshal.marshal( static_cast<uint8_t>( 'l' ) );
    }else{
        marshal.marshal( static_cast<uint8_t>( 'B' ) );
    }

    switch( type() ) {
    case MessageType::INVALID:
        return false;

    case MessageType::CALL:
        marshal.marshal( static_cast<uint8_t>( 1 ) );
        break;

    case MessageType::RETURN:
        marshal.marshal( static_cast<uint8_t>( 2 ) );
        mustHaveSerial = true;
        break;

    case MessageType::ERROR:
        marshal.marshal( static_cast<uint8_t>( 3 ) );
        mustHaveSerial = true;
        break;

    case MessageType::SIGNAL:
        marshal.marshal( static_cast<uint8_t>( 4 ) );
        break;
    }

    // Now marshal the flags
    marshal.marshal( m_priv->m_flags );

    // Marshal the protocol version
    marshal.marshal( static_cast<uint8_t>( 1 ) );

    // Marshal the length
    marshal.marshal( static_cast<uint32_t>( m_priv->m_body.size() ) );

    if( mustHaveSerial ) {
        // Make sure that we have a header for our serial and it is not 0
        if( serialHeader.type() == DataType::UINT32 ) {
            uint32_t tmpSerial = serialHeader.to_uint32();

            if( tmpSerial == 0 ) {
                SIMPLELOGGER_ERROR( LOGGER_NAME, "Unable to serialize message: invalid return serial provided!" );
                return false;
            }
        } else {
            SIMPLELOGGER_ERROR( LOGGER_NAME, "Unable to serialize message: reply serial required but not found!" );
            return false;
        }
    }

    marshal.marshal( serial );

    // Marshal our header array
    marshal.marshal( static_cast<uint32_t>( 0 ) ); // The size of the header array; we update this later

    for( const std::pair<const MessageHeaderFields, Variant>& entry : m_priv->m_headerMap ) {
        if( entry.second.type() == DataType::INVALID ) { continue; }

        marshal.align( 8 );
        marshal.marshal( header_field_to_int( entry.first ) );
        marshal.marshal( entry.second );
    }

    // The size of the header array is always at offset 12
    marshal.marshal_at_offset( 12, static_cast<uint32_t>( vec->size() ) - 16 );

    // Align the message data to an 8-byte boundary and add the data!
    marshal.align( 8 );

    for( const uint8_t& byte : m_priv->m_body ) {
        vec->push_back( byte );
    }

    if( !Validator::message_is_small_enough( vec ) ) {
        return false;
    }

    return true;
}

std::shared_ptr<Message> Message::create_from_data( uint8_t* data, uint32_t data_len, std::vector<int> fds ) {
    Demarshaling demarshal( data, data_len, Endianess::Big );
    uint8_t method_type;
    uint8_t flags;
    uint8_t protoVersion;
    uint32_t bodyLen;
    uint32_t serial;
    uint32_t arrayLen;
    std::shared_ptr<Message> retmsg;
    std::map<MessageHeaderFields, Variant> headerMap;
    Endianess msgEndian = Endianess::Big;
    std::vector<int> real_fds;

    if( demarshal.demarshal_uint8_t() == 'l' ) {
        demarshal.set_endianess( Endianess::Little );
        msgEndian = Endianess::Little;
    }

    method_type = demarshal.demarshal_uint8_t();
    flags = demarshal.demarshal_uint8_t();
    protoVersion = demarshal.demarshal_uint8_t();
    bodyLen = demarshal.demarshal_uint32_t();
    serial = demarshal.demarshal_uint32_t();
    arrayLen = demarshal.demarshal_uint32_t();

    while( demarshal.current_offset() < ( 12 + arrayLen ) ) {
        uint8_t key_demarshal;
        MessageHeaderFields key;
        Variant value;
        demarshal.align( 8 );
        key_demarshal = demarshal.demarshal_uint8_t();
        value = demarshal.demarshal_variant();

        key = int_to_header_field( key_demarshal );

        if( key == MessageHeaderFields::Invalid ) {
            std::ostringstream logmsg;
            logmsg << "Found invalid header field "
                << static_cast<int>( key )
                << " when parsing; ignoring.  Value: "
                << value;
            SIMPLELOGGER_WARN( LOGGER_NAME, logmsg.str() );
            continue;
        }

        if( key == MessageHeaderFields::Unix_FDs ) {
            int total_fds = value.to_uint32();

            for( int fd_num = 0; fd_num < total_fds; fd_num++ ) {
                real_fds.push_back( fds[ fd_num ] );
            }
        }

        headerMap[ key ] = value;
    }

    // Make sure we're aligned to an 8-byte boundary
    demarshal.align( 8 );

    switch( method_type ) {
    case 1:
        SIMPLELOGGER_TRACE( LOGGER_NAME, "Creating CallMessage from data" );
        retmsg = CallMessage::create();
        break;

    case 2:
        SIMPLELOGGER_TRACE( LOGGER_NAME, "Creating ReturnMessage from data" );
        retmsg = ReturnMessage::create();
        break;

    case 3:
        SIMPLELOGGER_TRACE( LOGGER_NAME, "Creating ErrorMessage from data" );
        retmsg = ErrorMessage::create();
        break;

    case 4:
        SIMPLELOGGER_TRACE( LOGGER_NAME, "Creating SignalMessage from data" );
        retmsg = SignalMessage::create();
        break;
    }

    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Message has " << real_fds.size() << " fds" );

    retmsg->m_priv->m_serial = serial;
    retmsg->m_priv->m_flags = flags;
    retmsg->m_priv->m_valid = true;
    retmsg->m_priv->m_headerMap = headerMap;
    retmsg->m_priv->m_endianess = msgEndian;
    retmsg->m_priv->m_body.reserve( bodyLen );
    retmsg->m_priv->m_filedescriptors = real_fds;

    for( uint32_t x = 0; x < bodyLen; x++ ) {
        retmsg->m_priv->m_body.push_back( demarshal.demarshal_uint8_t() );
    }

    {
        std::ostringstream debug_str;
        debug_str << "Following message created from the data: " << retmsg;
        SIMPLELOGGER_TRACE( LOGGER_NAME, debug_str.str() );
    }

    return retmsg;
}

void Message::append_signature( std::string toappend ) {
    DBus::Variant val = m_priv->m_headerMap[ MessageHeaderFields::Signature ];
    std::string newval;

    if( val.type() != DataType::INVALID ) {
        Signature sig = val.to_signature();
        newval += sig.str();
    }

    newval += toappend;
    m_priv->m_headerMap[ MessageHeaderFields::Signature ] = DBus::Variant( Signature( newval ) );
}

Variant Message::header_field( MessageHeaderFields field ) const {
    std::map<MessageHeaderFields, Variant>::const_iterator location =
        m_priv->m_headerMap.find( field );

    if( location != m_priv->m_headerMap.end() ) {
        return location->second;
    }

    return DBus::Variant();
}

void Message::clear_sig_and_data() {
    std::map<MessageHeaderFields, Variant>::const_iterator location =
        m_priv->m_headerMap.find( MessageHeaderFields::Signature );

    if( location != m_priv->m_headerMap.end() ) {
        m_priv->m_headerMap.erase( location );
    }

    m_priv->m_body.clear();
}

uint8_t Message::flags() const {
    return m_priv->m_flags;
}

void Message::set_flags( uint8_t flags ) {
    m_priv->m_flags = flags;
}

Variant Message::set_header_field( MessageHeaderFields field, Variant value ) {
    DBus::Variant retval = header_field( field );

    m_priv->m_headerMap[ field ] = value;

    return retval;
}

std::vector<uint8_t>* Message::body() {
    return &m_priv->m_body;
}

const std::vector<uint8_t>* Message::body() const {
    return &m_priv->m_body;
}

void Message::add_filedescriptor( int fd ) {
    m_priv->m_filedescriptors.push_back( fd );

    uint32_t fds_size = m_priv->m_filedescriptors.size();
    set_header_field( MessageHeaderFields::Unix_FDs, Variant( fds_size ) );
}

uint32_t Message::filedescriptors_size() const {
    return m_priv->m_filedescriptors.size();
}

DBus::Endianess Message::endianess() const {
    return m_priv->m_endianess;
}

int Message::filedescriptor_at_location( int location ) const {
    if( location < m_priv->m_filedescriptors.size() &&
        location >= 0 ) {
        return m_priv->m_filedescriptors[ location ];
    }

    return -1;
}

const std::vector<int>& Message::filedescriptors() const {
    return m_priv->m_filedescriptors;
}

std::ostream& operator <<( std::ostream& os, const DBus::Message* msg ) {
    os << "DBus::Message = [";

    switch( msg->type() ) {
    case DBus::MessageType::INVALID:
        os << "Invalid";
        break;

    case DBus::MessageType::CALL:
        os << "CallMessage";
        break;

    case DBus::MessageType::ERROR:
        os << "ErrorMessage";
        break;

    case DBus::MessageType::RETURN:
        os << "ReturnMessage";
        break;

    case DBus::MessageType::SIGNAL:
        os << "SignalMessage";
        break;
    }

    os << std::endl;
    os << "  Message length: " << msg->m_priv->m_body.size() << std::endl;
    os << "  Endianess: " << msg->m_priv->m_endianess << std::endl;
    os << "  Serial: " << msg->m_priv->m_serial << std::endl;
    os << "  Headers:" << std::endl;

    for( const std::pair<const MessageHeaderFields, DBus::Variant>& set : msg->m_priv->m_headerMap ) {
        os << "    ";

        switch( set.first ) {
        case MessageHeaderFields::Invalid:
            os << "!! Invalid message header has been stored !!";
            break;

        case MessageHeaderFields::Path:
            os << "Path: " << set.second.to_path();
            break;

        case MessageHeaderFields::Interface:
            os << "Interface: " << set.second.to_string();
            break;

        case MessageHeaderFields::Member:
            os << "Member: " << set.second.to_string();
            break;

        case MessageHeaderFields::Error_Name:
            os << "Error Name: " << set.second.to_string();
            break;

        case MessageHeaderFields::Reply_Serial:
            os << "Reply Serial: " << set.second.to_uint32();
            break;

        case MessageHeaderFields::Destination:
            os << "Destination: " << set.second.to_string();
            break;

        case MessageHeaderFields::Sender:
            os << "Sender: " << set.second.to_string();
            break;

        case MessageHeaderFields::Signature:
            os << "Signature: " << set.second.to_signature();
            break;

        case MessageHeaderFields::Unix_FDs:
            os << "# Unix FDs: " << set.second.to_uint32();
            break;
        }

        os << std::endl;
    }

    os << "]";

    return os;
}

}
