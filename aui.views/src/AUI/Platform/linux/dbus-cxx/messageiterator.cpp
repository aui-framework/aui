// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2009 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "messageiterator.h"
#include <cstring>
#include "filedescriptor.h"
#include "message.h"
#include "types.h"
#include "variant.h"
#include "dbus-cxx-private.h"

#include <unistd.h>
#include <fcntl.h>

static const char* LOGGER_NAME = "DBus.MessageIterator";

namespace DBus {

class MessageIterator::priv_data {
public:
    priv_data() : m_message( nullptr )
    {}

    struct SubiterInformation {
        SubiterInformation() :
            m_subiterDataType( DataType::INVALID ),
            m_arrayLastPosition( 0 ) {}

        DataType m_subiterDataType;
        uint32_t m_arrayLastPosition;
        Signature m_variantSignature;
    };

    const Message* m_message;
    std::shared_ptr<Demarshaling> m_demarshal;
    SignatureIterator m_signatureIterator;
    SubiterInformation m_subiterInfo;
};

MessageIterator::MessageIterator( DataType d,
    SignatureIterator sig,
    const Message* message,
    std::shared_ptr<Demarshaling> demarshal ) {
    m_priv = std::make_shared<priv_data>();
    m_priv->m_message = message;
    m_priv->m_demarshal = demarshal;
    m_priv->m_signatureIterator = sig;

    if( d == DataType::ARRAY ) {
        m_priv->m_subiterInfo.m_subiterDataType = d;
        uint32_t array_len = m_priv->m_demarshal->demarshal_uint32_t();
        m_priv->m_subiterInfo.m_arrayLastPosition = m_priv->m_demarshal->current_offset() + array_len;
        SIMPLELOGGER_TRACE_STDSTR( LOGGER_NAME,
                                   "Extracting array.  new position: " << m_priv->m_demarshal->current_offset()
                                   << " array len: " << array_len
                                   << " array end pos: " << m_priv->m_subiterInfo.m_arrayLastPosition);
    } else if( d == DataType::DICT_ENTRY || d == DataType::STRUCT ) {
        m_priv->m_demarshal->align( 8 );
        SIMPLELOGGER_TRACE_STDSTR( LOGGER_NAME,
                                   "Extracting DICT or STRUCT: aligning to 8 byte boundary.  Location: " << m_priv->m_demarshal->current_offset() );
    }

}

MessageIterator::MessageIterator():
    m_priv( std::make_shared<priv_data>() ) {
    m_priv->m_subiterInfo.m_subiterDataType = DataType::INVALID;
}

MessageIterator::MessageIterator( const Message& message ):
    m_priv( std::make_shared<priv_data>() ) {
    m_priv->m_message = &message;
    m_priv->m_demarshal = std::shared_ptr<Demarshaling>(
            new Demarshaling( m_priv->m_message->body()->data(),
                m_priv->m_message->body()->size(),
                m_priv->m_message->endianess() )
        );
    m_priv->m_signatureIterator = m_priv->m_message->signature().begin();
    m_priv->m_subiterInfo.m_subiterDataType = DataType::INVALID;
}

MessageIterator::MessageIterator( std::shared_ptr<Message> message ):
    m_priv( std::make_shared<priv_data>() ) {
    m_priv->m_message = message.get();
    m_priv->m_demarshal = std::shared_ptr<Demarshaling>(
            new Demarshaling( m_priv->m_message->body()->data(),
                m_priv->m_message->body()->size(),
                m_priv->m_message->endianess() )
        );
    m_priv->m_signatureIterator = m_priv->m_message->signature().begin();
    m_priv->m_subiterInfo.m_subiterDataType = DataType::INVALID;
}

const Message* MessageIterator::message() const {
    return m_priv->m_message;
}

void MessageIterator::invalidate() {
    m_priv->m_message = nullptr;
}

bool MessageIterator::is_valid() const {
    if( !( m_priv->m_message && m_priv->m_message->is_valid() ) ) { return false; }

    if( this->arg_type() == DataType::INVALID ) { return false; }

    if( m_priv->m_subiterInfo.m_subiterDataType == DataType::ARRAY ) {
        // We are in a subiter here, figure out if we're at the end of the array yet
        SIMPLELOGGER_TRACE_STDSTR( LOGGER_NAME,
                                   "Current offset: " << m_priv->m_demarshal->current_offset() << " last array pos: " << m_priv-> m_subiterInfo.m_arrayLastPosition  );
        if( m_priv->m_demarshal->current_offset() >= m_priv-> m_subiterInfo.m_arrayLastPosition ) {
            SIMPLELOGGER_TRACE_STDSTR( LOGGER_NAME,
                                       "Array extraction done.  new position: " << m_priv->m_demarshal->current_offset() );

            return false;
        }

        return true;
    }

    return true;
}

bool MessageIterator::has_next() const {
    if( this->is_valid() ) { return m_priv->m_signatureIterator.has_next(); }

    return false;
}

bool MessageIterator::next() {
    if( !this->is_valid() ) { return false; }

    // Check to see if we are a subiterator.  If we are, it depends on the type we are.
    // Arrays are valid until they can read no more data,
    // structs iterate over their types like normal,
    // and variants do ..  ?
    if( m_priv->m_subiterInfo.m_subiterDataType == DataType::ARRAY ) {
        // Question: should this try to advance the iterator by reading a value
        // and then just discarding it?
        return true;
    }

    bool result = m_priv->m_signatureIterator.next();

    if( !result || this->arg_type() == DataType::INVALID ) {
        this->invalidate();
        return false;
    }

    return true;
}

MessageIterator& MessageIterator::operator ++() {
    this->next();
    return ( *this );
}

MessageIterator MessageIterator::operator ++( int ) {
    MessageIterator temp_copy;
    temp_copy = *this;
    ++( *this );
    return temp_copy;
}

bool MessageIterator::operator==( const MessageIterator& other ) {
    //TODO finish this method
    return ( m_priv->m_message == other.m_priv->m_message );
}

DataType MessageIterator::arg_type() const {
    return m_priv->m_signatureIterator.type();
}

DataType MessageIterator::element_type() const {
    if( this->arg_type() != DataType::ARRAY ) {
        return DataType::INVALID;
    }

    return m_priv->m_signatureIterator.element_type();
}

bool MessageIterator::is_fixed() const {
    TypeInfo t( arg_type() );
    return t.is_fixed();
}

bool MessageIterator::is_container() const {
    TypeInfo t( arg_type() );
    return t.is_container();
}

bool MessageIterator::is_array() const {
    return this->arg_type() == DataType::ARRAY;
}

bool MessageIterator::is_dict() const {
    return this->is_array() && this->element_type() == DataType::DICT_ENTRY;
}

MessageIterator MessageIterator::recurse() {
    if( !this->is_container() ) { return MessageIterator(); }

    MessageIterator iter( m_priv->m_signatureIterator.type(),
        m_priv->m_signatureIterator.recurse(),
        m_priv->m_message,
        m_priv->m_demarshal );

    return iter;
}

std::string MessageIterator::signature() const {
    return m_priv->m_signatureIterator.signature();
}

MessageIterator::operator bool() {
    // TODO check for invalid
    switch( this->arg_type() ) {
    case DataType::BYTE:    return ( uint8_t )( *this );

    case DataType::BOOLEAN: return get_bool();

    case DataType::INT16:   return ( int16_t )( *this );

    case DataType::UINT16:  return ( uint16_t )( *this );

    case DataType::INT32:   return ( int32_t )( *this );

    case DataType::UINT32:  return ( uint32_t )( *this );

    case DataType::INT64:   return ( int64_t )( *this );

    case DataType::UINT64:  return ( uint64_t )( *this );

    case DataType::DOUBLE:  return ( double )( *this );

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to boolean value" );
    }
}

MessageIterator::operator uint8_t() {
    // TODO check for invalid
    switch( this->arg_type() ) {
    case DataType::BYTE:    return get_uint8();

    case DataType::BOOLEAN: return ( bool )( *this );

    case DataType::INT16:   return ( int16_t )( *this );

    case DataType::UINT16:  return ( int16_t )( *this );

    case DataType::INT32:   return ( int32_t )( *this );

    case DataType::UINT32:  return ( uint32_t )( *this );

    case DataType::INT64:   return ( int64_t )( *this );

    case DataType::UINT64:  return ( uint64_t )( *this );

    case DataType::DOUBLE:  return ( double )( *this );

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to numeric value" );
    }
}

MessageIterator::operator uint16_t() {
    // TODO check for invalid
    switch( this->arg_type() ) {
    case DataType::BYTE:    return ( uint8_t )( *this );

    case DataType::BOOLEAN: return ( bool )( *this );

    case DataType::INT16:   return ( int16_t )( *this );

    case DataType::UINT16:  return get_uint16();

    case DataType::INT32:   return ( int32_t )( *this );

    case DataType::UINT32:  return ( uint32_t )( *this );

    case DataType::INT64:   return ( int64_t )( *this );

    case DataType::UINT64:  return ( uint64_t )( *this );

    case DataType::DOUBLE:  return ( double )( *this );

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to numeric value" );
    }
}

MessageIterator::operator int16_t() {
    // TODO check for invalid
    switch( this->arg_type() ) {
    case DataType::BYTE:    return ( uint8_t )( *this );

    case DataType::BOOLEAN: return ( bool )( *this );

    case DataType::INT16:   return get_int16();

    case DataType::UINT16:  return ( uint16_t )( *this );

    case DataType::INT32:   return ( int32_t )( *this );

    case DataType::UINT32:  return ( uint32_t )( *this );

    case DataType::INT64:   return ( int64_t )( *this );

    case DataType::UINT64:  return ( uint64_t )( *this );

    case DataType::DOUBLE:  return ( double )( *this );

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to numeric value" );
    }
}

MessageIterator::operator uint32_t() {
    // TODO check for invalid
    switch( this->arg_type() ) {
    case DataType::BYTE:    return ( uint8_t )( *this );

    case DataType::BOOLEAN: return ( bool )( *this );

    case DataType::INT16:   return ( int16_t )( *this );

    case DataType::UINT16:  return ( uint16_t )( *this );

    case DataType::INT32:   return ( int32_t )( *this );

    case DataType::UINT32:  return get_uint32();

    case DataType::INT64:   return ( int64_t )( *this );

    case DataType::UINT64:  return ( uint64_t )( *this );

    case DataType::DOUBLE:  return ( double )( *this );

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to numeric value" );
    }
}

MessageIterator::operator int32_t() {
    // TODO check for invalid
    switch( this->arg_type() ) {
    case DataType::BYTE:    return ( uint8_t )( *this );

    case DataType::BOOLEAN: return ( bool )( *this );

    case DataType::INT16:   return ( int16_t )( *this );

    case DataType::UINT16:  return ( uint16_t )( *this );

    case DataType::INT32:   return get_int32();

    case DataType::UINT32:  return ( uint32_t )( *this );

    case DataType::INT64:   return ( int64_t )( *this );

    case DataType::UINT64:  return ( uint64_t )( *this );

    case DataType::DOUBLE:  return ( double )( *this );

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to numeric value" );
    }
}

MessageIterator::operator uint64_t() {
    // TODO check for invalid
    switch( this->arg_type() ) {
    case DataType::BYTE:    return ( uint8_t )( *this );

    case DataType::BOOLEAN: return ( bool )( *this );

    case DataType::INT16:   return ( int16_t )( *this );

    case DataType::UINT16:  return ( uint16_t )( *this );

    case DataType::INT32:   return ( int32_t )( *this );

    case DataType::UINT32:  return ( uint32_t )( *this );

    case DataType::INT64:   return ( int64_t )( *this );

    case DataType::UINT64:  return get_uint64();

    case DataType::DOUBLE:  return ( double )( *this );

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to numeric value" );
    }
}

MessageIterator::operator int64_t() {
    // TODO check for invalid
    switch( this->arg_type() ) {
    case DataType::BYTE:    return ( uint8_t )( *this );

    case DataType::BOOLEAN: return ( bool )( *this );

    case DataType::INT16:   return ( int16_t )( *this );

    case DataType::UINT16:  return ( uint16_t )( *this );

    case DataType::INT32:   return ( int32_t )( *this );

    case DataType::UINT32:  return ( uint32_t )( *this );

    case DataType::INT64:   return get_int64();

    case DataType::UINT64:  return ( uint64_t )( *this );

    case DataType::DOUBLE:  return ( double )( *this );

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to numeric value" );
    }
}

MessageIterator::operator double() {
    // TODO check for invalid
    switch( this->arg_type() ) {
    case DataType::BYTE:    return ( uint8_t )( *this );

    case DataType::BOOLEAN: return ( bool )( *this );

    case DataType::INT16:   return ( int16_t )( *this );

    case DataType::UINT16:  return ( uint16_t )( *this );

    case DataType::INT32:   return ( int32_t )( *this );

    case DataType::UINT32:  return ( uint32_t )( *this );

    case DataType::INT64:   return ( int64_t )( *this );

    case DataType::UINT64:  return ( uint64_t )( *this );

    case DataType::DOUBLE:  return get_double();

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to numeric value" );
    }
}

MessageIterator::operator std::string() {
    switch( this->arg_type() ) {
    case DataType::STRING:
    case DataType::OBJECT_PATH:
        return get_string();

    case DataType::SIGNATURE:
        return get_signature();

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: extracting non-string type to std::string" );
    }
}

MessageIterator::operator std::shared_ptr<FileDescriptor>() {
    switch( this->arg_type() ) {
    case DataType::UNIX_FD: return get_filedescriptor();

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting non-numeric type to numeric value" );
    }
}

MessageIterator::operator Variant() {
    switch( this->arg_type() ) {
    case DataType::VARIANT: return get_variant();

    default:
        throw ErrorInvalidTypecast( "MessageIterator:: casting invalid type to variant" );
    }
}

//  MessageIterator::operator Signature(){
//    switch ( this->arg_type() )
//    {
//      case DataType::SIGNATURE: return get_signature();
//      default:
//        throw ErrorInvalidTypecast("MessageIterator:: casting invalid type to signature");
//    }
//  }

bool MessageIterator::get_bool() {
    if( this->arg_type() != DataType::BOOLEAN ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting bool and type is not DataType::BOOLEAN" );
    }

    return m_priv->m_demarshal->demarshal_boolean();
}

uint8_t MessageIterator::get_uint8() {
    if( this->arg_type() != DataType::BYTE ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting uint8_t and type is not DataType::BYTE" );
    }

    return m_priv->m_demarshal->demarshal_uint8_t();
}

int16_t MessageIterator::get_int16() {
    if( this->arg_type() != DataType::INT16 ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting int16_t and type is not DataType::INT16" );
    }

    return m_priv->m_demarshal->demarshal_int16_t();
}

uint16_t MessageIterator::get_uint16() {
    if( this->arg_type() != DataType::UINT16 ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting uint16_t and type is not DataType::UINT16" );
    }

    return m_priv->m_demarshal->demarshal_uint16_t();
}

int32_t MessageIterator::get_int32() {
    if( this->arg_type() != DataType::INT32 ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting int32_t and type is not DataType::INT32" );
    }

    return m_priv->m_demarshal->demarshal_int32_t();
}

uint32_t MessageIterator::get_uint32() {
    if( this->arg_type() != DataType::UINT32 ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting uint32_t and type is not DataType::UINT32" );
    }

    return m_priv->m_demarshal->demarshal_uint32_t();
}

int64_t MessageIterator::get_int64() {
    if( this->arg_type() != DataType::INT64 ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting int64_t and type is not DataType::INT64" );
    }

    return m_priv->m_demarshal->demarshal_int64_t();
}

uint64_t MessageIterator::get_uint64() {
    if( this->arg_type() != DataType::UINT64 ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting uint64_t and type is not DataType::UINT64" );
    }

    return m_priv->m_demarshal->demarshal_uint64_t();
}

double MessageIterator::get_double() {
    if( this->arg_type() != DataType::DOUBLE ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting double and type is not DataType::DOUBLE" );
    }

    return m_priv->m_demarshal->demarshal_double();
}

std::string MessageIterator::get_string() {
    if( !( this->arg_type() == DataType::STRING || this->arg_type() == DataType::OBJECT_PATH || this->arg_type() == DataType::SIGNATURE ) ) {
        throw ErrorInvalidTypecast( "MessageIterator: getting char* and type is not one of DataType::STRING, DataType::OBJECT_PATH or DataType::SIGNATURE" );
    }

    if( this->arg_type() == DataType::SIGNATURE ) {
        return m_priv->m_demarshal->demarshal_signature();
    }

    return m_priv->m_demarshal->demarshal_string();
}

std::shared_ptr<FileDescriptor> MessageIterator::get_filedescriptor() {
    std::shared_ptr<FileDescriptor> fd;
    int32_t fd_location = m_priv->m_demarshal->demarshal_int32_t();

    int raw_fd = m_priv->m_message->filedescriptor_at_location( fd_location );

    if( raw_fd < 0 ) {
        return FileDescriptor::create( -1 );
    }

    // We need to duplicate, as the filedescriptors will be closed when the message is destructed
    int new_fd = fcntl( raw_fd, F_DUPFD, 3 );

    if( new_fd < 0 ) {
        return FileDescriptor::create( -1 );
    }

    return FileDescriptor::create( new_fd );
}

Variant MessageIterator::get_variant() {
    DBus::Signature sig = get_signature();
    const std::vector<int> descriptors = m_priv->m_message->filedescriptors();

    return Variant::createFromDemarshal( sig, m_priv->m_demarshal, descriptors, 0 );
}

Signature MessageIterator::get_signature() {
    return m_priv->m_demarshal->demarshal_signature();
}

void MessageIterator::align( int alignment ) {
    m_priv->m_demarshal->align( alignment );
}

SignatureIterator MessageIterator::signature_iterator() {
    return m_priv->m_signatureIterator;
}

}

