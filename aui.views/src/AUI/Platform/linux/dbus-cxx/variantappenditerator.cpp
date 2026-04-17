// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "variantappenditerator.h"
#include "variant.h"
#include "marshaling.h"
#include <AUI/Platform/linux/dbus-cxx/types.h>
#include <AUI/Platform/linux/dbus-cxx/marshaling.h>
#include <AUI/Platform/linux/dbus-cxx/validator.h>
#include "utility.h"

using DBus::priv::VariantAppendIterator;

class VariantAppendIterator::priv_data {
public:
    priv_data( Variant* variant ) :
        m_variant( variant ),
        m_subiter( nullptr ),
        m_currentContainer( ContainerType::None ),
        m_arrayAlignment( 0 )
    {}

    Variant* m_variant;
    VariantAppendIterator* m_subiter;
    ContainerType m_currentContainer;
    std::vector<uint8_t> m_workingBuffer;
    int32_t m_arrayAlignment;
    Marshaling m_marshaling;
};

VariantAppendIterator::VariantAppendIterator( Variant* variant ):
    m_priv( std::make_shared<priv_data>( variant ) ) {
    m_priv->m_marshaling = Marshaling( &variant->m_marshaled, DBus::default_endianess());
}

VariantAppendIterator::VariantAppendIterator( Variant* variant, ContainerType t ) :
    m_priv( std::make_shared<priv_data>( variant ) ) {
    m_priv->m_currentContainer = t;
    m_priv->m_marshaling = Marshaling( &m_priv->m_workingBuffer, DBus::default_endianess() );
}

VariantAppendIterator::~VariantAppendIterator() {
    delete m_priv->m_subiter;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const bool& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const uint8_t& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const int16_t& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const uint16_t& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const int32_t& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const uint32_t& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const int64_t& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const uint64_t& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const double& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const char* v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const std::string& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const Signature& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const Path& v ) {
    m_priv->m_marshaling.marshal( v );

    return *this;
}

VariantAppendIterator& VariantAppendIterator::operator<<( const Variant& v ){
    if( m_priv->m_subiter ) { this->close_container(); }

    // The variant body is marshaled, but first we need to marshal the signature of the variant
    m_priv->m_marshaling.marshal( v.m_signature );
    m_priv->m_marshaling.align( v.data_alignment() );
    TypeInfo ti( v.m_currentType );
    Signature sig = v.m_signature;

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

    return *this;
}

bool VariantAppendIterator::open_container( ContainerType t, const std::string& sig ) {
    std::string signature;
    int32_t array_align = 0;

    if( m_priv->m_subiter ) { this->close_container(); }

    if( t == ContainerType::ARRAY ) {
        Signature tmpSig( sig );
        SignatureIterator tmpSigIter = tmpSig.begin();
        TypeInfo ti( tmpSigIter.type() );
        array_align = ti.alignment();
    }

    m_priv->m_subiter = new VariantAppendIterator( m_priv->m_variant, t );
    m_priv->m_subiter->m_priv->m_arrayAlignment = array_align;

    return true;
}

bool VariantAppendIterator::close_container( ) {
    if( ! m_priv->m_subiter ) { return false; }

    switch( m_priv->m_subiter->m_priv->m_currentContainer ) {
    case ContainerType::None: return false;

    case ContainerType::ARRAY: {
        uint32_t arraySize = static_cast<uint32_t>( m_priv->m_subiter->m_priv->m_workingBuffer.size() );

        if( arraySize > Validator::maximum_array_size() ) {
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

VariantAppendIterator* VariantAppendIterator::sub_iterator() {
    return m_priv->m_subiter;
}
