// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009 by Rick L. Vinyard, Jr.                            *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "messageappenditerator.h"
#include <AUI/Platform/linux/dbus-cxx/variant.h>
#include <AUI/Platform/linux/dbus-cxx/message.h>
#include <any>
#include <stdint.h>
#include <limits>
#include <cstring>
#include "enums.h"
#include "filedescriptor.h"
#include "message.h"
#include "signature.h"
#include "types.h"
#include "validator.h"
#include "utility.h"

#include <fcntl.h>
#include <unistd.h>

namespace DBus {

class MessageAppendIterator::priv_data {
public:
    priv_data() :
        m_message( nullptr ),
        m_subiter( nullptr ),
        m_currentContainer( ContainerType::None ),
        m_arrayAlignment( 0 ) {}

    Marshaling m_marshaling;
    Message* m_message;
    MessageAppendIterator* m_subiter;
    std::vector<uint8_t> m_workingBuffer;
    ContainerType m_currentContainer;
    int32_t m_arrayAlignment;
};

MessageAppendIterator::MessageAppendIterator( ContainerType container ) {
    m_priv = std::make_shared<priv_data>();
    m_priv->m_currentContainer = container;
}

MessageAppendIterator::MessageAppendIterator( Message& message, ContainerType container ) {
    m_priv = std::make_shared<priv_data>();
    m_priv->m_marshaling = Marshaling( message.body(), DBus::default_endianess() );
    m_priv->m_message = &message;
    m_priv->m_currentContainer = container;

    if( container != ContainerType::None ) {
        m_priv->m_marshaling = Marshaling( &m_priv->m_workingBuffer, DBus::default_endianess() );
    }
}

MessageAppendIterator::MessageAppendIterator( std::shared_ptr<Message> message, ContainerType container ) {
    m_priv = std::make_shared<priv_data>();
    m_priv->m_message = message.get();
    m_priv->m_currentContainer = container;

    if( message ) {
        m_priv->m_marshaling = Marshaling( message->body(), DBus::default_endianess() );
    }

    if( container != ContainerType::None ) {
        m_priv->m_marshaling = Marshaling( &m_priv->m_workingBuffer, DBus::default_endianess() );
    }
}

MessageAppendIterator::~MessageAppendIterator() {
    //     if ( m_subiter ) this->close_container();
}

void MessageAppendIterator::invalidate() {
    m_priv->m_message = nullptr;
    m_priv->m_subiter = nullptr;
}

bool MessageAppendIterator::is_valid() const {
    if( !( m_priv->m_message && m_priv->m_message->is_valid() ) ) { return false; }

    return true;
}

MessageAppendIterator::operator bool() const {
    return this->is_valid();
}


MessageAppendIterator& MessageAppendIterator::operator<<( const bool& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const uint8_t& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const int16_t& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const uint16_t& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const int32_t& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const uint32_t& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const int64_t& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const uint64_t& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const double& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const char* v ) {
    uint32_t len = strlen( v ) & UINT32_MAX;

    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( std::string() ) );
    }

    m_priv->m_marshaling.marshal( std::string( v, len ) );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const std::string& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( std::string() ) );
    }

    m_priv->m_marshaling.marshal( v );
    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const Signature& v ) {
    if( !this->is_valid() ) { return *this; }

    std::string realSig = v.str();

    if( realSig.length() > UINT8_MAX ) {
        m_priv->m_message->invalidate();
        return *this;
    }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( static_cast<uint8_t>( realSig.length() & 0xFF ) );

    for( const char& c : realSig ) {
        m_priv->m_marshaling.marshal( static_cast<uint8_t>( c ) );
    }

    m_priv->m_marshaling.marshal( static_cast<uint8_t>( 0 ) );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const Path& v ) {
    if( !this->is_valid() ) { return *this; }

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    m_priv->m_marshaling.marshal( v );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const std::shared_ptr<FileDescriptor> v ) {
    int raw_fd;
    uint32_t array_location;

    if( !this->is_valid() ) { return *this; }

    if( !v ) { return *this; }

    raw_fd = v->descriptor();

    if( m_priv->m_currentContainer == ContainerType::None ) {
        m_priv->m_message->append_signature( signature( v ) );
    }

    // Duplicate the FD so that when we return, it may be closed by the library user.
    // See documentation for dbus_message_iter_append_basic
    int new_fd = fcntl( raw_fd, F_DUPFD_CLOEXEC, 3 );

    if( new_fd < 0 ) {
        m_priv->m_message->invalidate();
        return *this;
    }

    m_priv->m_message->add_filedescriptor( new_fd );
    array_location = m_priv->m_message->filedescriptors_size() - 1;
    m_priv->m_marshaling.marshal( array_location );

    return *this;
}

MessageAppendIterator& MessageAppendIterator::operator<<( const Variant& v ) {
    if( !this->is_valid() ) { return *this; }

    if( v.type() == DataType::INVALID ) {
        return *this;
    }

    this->open_container( ContainerType::VARIANT, v.signature() );
    DBus::Signature sig = v.signature();
    m_priv->m_marshaling.marshal( sig );
    m_priv->m_marshaling.align( v.data_alignment() );

    // Note that when the Variant marshals a DICT, it will make sure to align
    // the DICT_ENTRY on an 8-byte boundary, which results in padding being
    // inserted into the data stream.  This padding is not always needed,
    // so we remove the padding and then do the alignment here.
    const std::vector<uint8_t>* dataToMarshal = v.marshaled();
    bool isDict = false;
    if( v.type() == DataType::ARRAY ){
        // determine if we have a dict or not.  If we do, remove padding
        for( DBus::SignatureIterator sig_it = sig.begin().recurse();
             sig_it != sig.end();
             sig_it++ ){
            if( sig_it.type() == DataType::DICT_ENTRY ){
                isDict = true;
                break;
            }
        }

    }

    for( size_t x = 0; x < dataToMarshal->size(); x++ ) {
        if( isDict &&
                (x == 4 || x == 5 || x == 6 || x == 7) ){
            // Ignore the 4 padding bytes that the variant inserts
            continue;
        }
        if( isDict && x == 8 ){
            // make sure to align for all of the dict entries
            m_priv->m_marshaling.align( 8 );
        }
        m_priv->m_marshaling.marshal( (*dataToMarshal)[x] );
    }

    this->close_container();

    return *this;
}


bool MessageAppendIterator::open_container( ContainerType t, const std::string& sig ) {
    std::string signature;
    int32_t array_align = 0;

    switch( t ) {
    case ContainerType::None:
        break;

    case ContainerType::STRUCT:
        signature.append( "(" );
        signature.append( sig );
        signature.append( ")" );
        break;

    case ContainerType::ARRAY:
        signature.append( "a" );
        signature.append( sig );
        // The size will get marshalled once we close the container
        {
            Signature tmpSig( sig );
            SignatureIterator tmpSigIter = tmpSig.begin();
            TypeInfo ti( tmpSigIter.type() );
            array_align = ti.alignment();
        }
        break;

    case ContainerType::VARIANT:
        signature.append( "v" );
        break;

    case ContainerType::DICT_ENTRY:
        break;
    }

    if( m_priv->m_subiter ) { this->close_container(); }

    if( m_priv->m_message ) {
        if( m_priv->m_currentContainer == ContainerType::None ) {
            m_priv->m_message->append_signature( signature );
        }

        m_priv->m_subiter = new MessageAppendIterator( *m_priv->m_message, t );
        m_priv->m_subiter->m_priv->m_arrayAlignment = array_align;
    } else {
        m_priv->m_subiter = new MessageAppendIterator( t );
    }

    return true;
}

bool MessageAppendIterator::close_container( ) {
    if( ! m_priv->m_subiter ) { return false; }

    switch( m_priv->m_subiter->m_priv->m_currentContainer ) {
    case ContainerType::None: return false;

    case ContainerType::ARRAY: {
        uint32_t arraySize = static_cast<uint32_t>( m_priv->m_subiter->m_priv->m_workingBuffer.size() );

        if( arraySize > Validator::maximum_array_size() ) {
            m_priv->m_message->invalidate();
            return true;
        }

        m_priv->m_marshaling.marshal( arraySize );
        m_priv->m_marshaling.align( m_priv->m_subiter->m_priv->m_arrayAlignment );
    }
    break;

    case ContainerType::DICT_ENTRY:
    case ContainerType::STRUCT:
        m_priv->m_marshaling.align( 8 );
        break;

    case ContainerType::VARIANT:
        break;
    }

    for( const uint8_t& dataByte : m_priv->m_subiter->m_priv->m_workingBuffer ) {
        m_priv->m_marshaling.marshal( dataByte );
    }

    delete m_priv->m_subiter;
    m_priv->m_subiter = nullptr;
    return true;
}

MessageAppendIterator* MessageAppendIterator::sub_iterator() {
    return m_priv->m_subiter;
}

}

