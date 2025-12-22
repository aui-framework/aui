// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/variantiterator.h>
#include <AUI/Platform/linux/dbus-cxx/demarshaling.h>
#include <AUI/Platform/linux/dbus-cxx/dbus-cxx-private.h>
#include "utility.h"

static const char* LOGGER_NAME = "DBus.VariantIterator";

using DBus::priv::VariantIterator;

class VariantIterator::priv_data {
public:
    priv_data() : m_variant( nullptr )
    {}

    struct SubiterInformation {
        SubiterInformation() :
            m_subiterDataType( DataType::INVALID ),
            m_arrayLastPosition( 0 ) {}

        DataType m_subiterDataType;
        uint32_t m_arrayLastPosition;
        Signature m_variantSignature;
    };

    const Variant* m_variant;
    std::shared_ptr<Demarshaling> m_demarshal;
    SignatureIterator m_signatureIterator;
    SubiterInformation m_subiterInfo;
};

VariantIterator::VariantIterator() {
    m_priv = std::make_shared<priv_data>();
}

VariantIterator::VariantIterator( const Variant* variant ) {
    m_priv = std::make_shared<priv_data>();
    m_priv->m_variant = variant;
    m_priv->m_demarshal = std::make_shared<Demarshaling>( variant->m_marshaled.data(), variant->m_marshaled.size(), DBus::default_endianess() );
    m_priv->m_signatureIterator = variant->signature().begin();
}

VariantIterator::VariantIterator( DataType d,
    SignatureIterator sig,
    const Variant* variant,
    std::shared_ptr<Demarshaling> demarshal ) {
    m_priv = std::make_shared<priv_data>();
    m_priv->m_variant = variant;
    m_priv->m_demarshal = demarshal;
    m_priv->m_signatureIterator = sig;

    if( d == DataType::ARRAY ) {
        uint32_t array_size = m_priv->m_demarshal->demarshal_uint32_t();
        m_priv->m_subiterInfo.m_subiterDataType = d;
        m_priv->m_subiterInfo.m_arrayLastPosition = m_priv->m_demarshal->current_offset() + array_size;
    } else if( d == DataType::VARIANT ) {
        Signature demarshaled_sig = demarshal->demarshal_signature();
        m_priv->m_subiterInfo.m_variantSignature = demarshaled_sig;
        m_priv->m_signatureIterator = demarshaled_sig.begin();
    } else if( d == DataType::DICT_ENTRY || d == DataType::STRUCT ) {
        m_priv->m_demarshal->align( 8 );
    }
}

void VariantIterator::invalidate() {
    m_priv->m_variant = nullptr;
}

bool VariantIterator::is_valid() const {
    if( !m_priv->m_variant ) { return false; }

    if( this->arg_type() == DataType::INVALID ) { return false; }

    if( m_priv->m_subiterInfo.m_subiterDataType == DataType::ARRAY ) {
        // We are in a subiter here, figure out if we're at the end of the array yet
        if( m_priv->m_demarshal->current_offset() >= m_priv-> m_subiterInfo.m_arrayLastPosition ) {
            return false;
        }

        return true;
    }

    return true;
}

bool VariantIterator::has_next() const {
    if( this->is_valid() ) { return m_priv->m_signatureIterator.has_next(); }

    return false;
}

bool VariantIterator::next() {
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

VariantIterator& VariantIterator::operator ++() {
    this->next();
    return ( *this );
}

VariantIterator VariantIterator::operator ++( int ) {
    VariantIterator temp_copy;
    temp_copy = *this;
    ++( *this );
    return temp_copy;
}

DBus::DataType VariantIterator::arg_type() const {
    return m_priv->m_signatureIterator.type();
}

DBus::DataType VariantIterator::element_type() const {
    if( this->arg_type() != DataType::ARRAY ) {
        return DataType::INVALID;
    }

    return m_priv->m_signatureIterator.element_type();
}

VariantIterator VariantIterator::recurse() {
    if( !this->is_container() ) { return VariantIterator(); }

    VariantIterator iter( m_priv->m_signatureIterator.type(),
        m_priv->m_signatureIterator.recurse(),
        m_priv->m_variant,
        m_priv->m_demarshal );

    return iter;
}

//std::string VariantIterator::signature() const
//{
//  return m_priv->m_signatureIterator.signature();
//}

bool VariantIterator::is_fixed() const {
    TypeInfo t( arg_type() );
    return t.is_fixed();
}

bool VariantIterator::is_container() const {
    TypeInfo t( arg_type() );
    return t.is_container();
}

bool VariantIterator::is_array() const {
    return this->arg_type() == DataType::ARRAY;
}

bool VariantIterator::is_dict() const {
    return this->is_array() && this->element_type() == DataType::DICT_ENTRY;
}

bool VariantIterator::is_struct() const {
    return arg_type() == DataType::STRUCT;
}

VariantIterator::operator bool() {
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
        throw ErrorInvalidTypecast( "VariantIterator:: casting non-numeric type to boolean value" );
    }
}

VariantIterator::operator uint8_t() {
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
        throw ErrorInvalidTypecast( "VariantIterator:: casting non-numeric type to numeric value" );
    }
}

VariantIterator::operator uint16_t() {
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
        throw ErrorInvalidTypecast( "VariantIterator:: casting non-numeric type to numeric value" );
    }
}

VariantIterator::operator int16_t() {
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
        throw ErrorInvalidTypecast( "VariantIterator:: casting non-numeric type to numeric value" );
    }
}

VariantIterator::operator uint32_t() {
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
        throw ErrorInvalidTypecast( "VariantIterator:: casting non-numeric type to numeric value" );
    }
}

VariantIterator::operator int32_t() {
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
        throw ErrorInvalidTypecast( "VariantIterator:: casting non-numeric type to numeric value" );
    }
}

VariantIterator::operator uint64_t() {
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
        throw ErrorInvalidTypecast( "VariantIterator:: casting non-numeric type to numeric value" );
    }
}

VariantIterator::operator int64_t() {
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
        throw ErrorInvalidTypecast( "VariantIterator:: casting non-numeric type to numeric value" );
    }
}

VariantIterator::operator double() {
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
        throw ErrorInvalidTypecast( "VariantIterator:: casting non-numeric type to numeric value" );
    }
}

VariantIterator::operator std::string() {
    switch( this->arg_type() ) {
    case DataType::STRING:
    case DataType::OBJECT_PATH:
        return get_string();

    case DataType::SIGNATURE:
        return get_signature();

    default:
        throw ErrorInvalidTypecast( "VariantIterator:: extracting non-string type to std::string" );
    }
}

VariantIterator::operator DBus::Variant() {
    switch( this->arg_type() ) {
    case DataType::VARIANT: return get_variant();

    default:
        throw ErrorInvalidTypecast( "VariantIterator: casting invalid type to variant" );
    }
}

bool VariantIterator::get_bool() {
    if( this->arg_type() != DataType::BOOLEAN ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting bool and type is not DataType::BOOLEAN" );
    }

    return m_priv->m_demarshal->demarshal_boolean();
}

uint8_t VariantIterator::get_uint8() {
    if( this->arg_type() != DataType::BYTE ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting uint8_t and type is not DataType::BYTE" );
    }

    return m_priv->m_demarshal->demarshal_uint8_t();
}

int16_t VariantIterator::get_int16() {
    if( this->arg_type() != DataType::INT16 ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting int16_t and type is not DataType::INT16" );
    }

    return m_priv->m_demarshal->demarshal_int16_t();
}

uint16_t VariantIterator::get_uint16() {
    if( this->arg_type() != DataType::UINT16 ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting uint16_t and type is not DataType::UINT16" );
    }

    return m_priv->m_demarshal->demarshal_uint16_t();
}

int32_t VariantIterator::get_int32() {
    if( this->arg_type() != DataType::INT32 ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting int32_t and type is not DataType::INT32" );
    }

    return m_priv->m_demarshal->demarshal_int32_t();
}

uint32_t VariantIterator::get_uint32() {
    if( this->arg_type() != DataType::UINT32 ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting uint32_t and type is not DataType::UINT32" );
    }

    return m_priv->m_demarshal->demarshal_uint32_t();
}

int64_t VariantIterator::get_int64() {
    if( this->arg_type() != DataType::INT64 ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting int64_t and type is not DataType::INT64" );
    }

    return m_priv->m_demarshal->demarshal_int64_t();
}

uint64_t VariantIterator::get_uint64() {
    if( this->arg_type() != DataType::UINT64 ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting uint64_t and type is not DataType::UINT64" );
    }

    return m_priv->m_demarshal->demarshal_uint64_t();
}

double VariantIterator::get_double() {
    if( this->arg_type() != DataType::DOUBLE ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting double and type is not DataType::DOUBLE" );
    }

    return m_priv->m_demarshal->demarshal_double();
}

std::string VariantIterator::get_string() {
    if( !( this->arg_type() == DataType::STRING || this->arg_type() == DataType::OBJECT_PATH || this->arg_type() == DataType::SIGNATURE ) ) {
        throw ErrorInvalidTypecast( "VariantIterator: getting char* and type is not one of DataType::STRING, DataType::OBJECT_PATH or DataType::SIGNATURE" );
    }

    if( this->arg_type() == DataType::SIGNATURE ) {
        return m_priv->m_demarshal->demarshal_signature();
    }

    return m_priv->m_demarshal->demarshal_string();
}

DBus::Variant VariantIterator::get_variant() {
    Signature sig = m_priv->m_demarshal->demarshal_signature();
    std::vector<int> descriptors;

    return DBus::Variant::createFromDemarshal(sig, m_priv->m_demarshal, descriptors, 0 );
}

DBus::Signature VariantIterator::get_signature() {
    return m_priv->m_demarshal->demarshal_signature();
}

VariantIterator& VariantIterator::operator>>( Variant& v ){
    v = operator Variant();
    this->next();
    return *this;
}
