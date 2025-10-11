// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "demarshaling.h"
#include <cstring>
#include <stdint.h>
#include <cassert>

using DBus::Demarshaling;

class Demarshaling::priv_data {
public:
    priv_data() :
        m_data( nullptr ),
        m_dataLen( 0 ),
        m_dataPos( 0 ),
        m_endian( Endianess::Big ) {}

    const uint8_t* m_data;
    uint32_t m_dataLen;
    uint32_t m_dataPos;
    Endianess m_endian;
};

Demarshaling::Demarshaling() {
    m_priv = std::make_unique<priv_data>();
}

Demarshaling::Demarshaling( const uint8_t* data, uint32_t dataLen, Endianess endian ) {
    m_priv = std::make_unique<priv_data>();
    m_priv->m_data = data;
    m_priv->m_dataLen = dataLen;
    m_priv->m_endian = endian;
}

Demarshaling::~Demarshaling() {
}

uint8_t Demarshaling::demarshal_uint8_t() {
    is_valid( 1 );
    return m_priv->m_data[m_priv->m_dataPos++];
}

bool Demarshaling::demarshal_boolean() {
    int32_t val = demarshal_int32_t();

    if( val ) {
        return true;
    }

    return false;
}

int16_t Demarshaling::demarshal_int16_t() {
    if( m_priv->m_endian == Endianess::Little ) {
        return demarshalShortLittle();
    } else {
        return demarshalShortBig();
    }
}

uint16_t Demarshaling::demarshal_uint16_t() {
    if( m_priv->m_endian == Endianess::Little ) {
        return static_cast<uint16_t>( demarshalShortLittle() );
    } else {
        return static_cast<uint16_t>( demarshalShortBig() );
    }
}

int32_t Demarshaling::demarshal_int32_t() {
    if( m_priv->m_endian == Endianess::Little ) {
        return demarshalIntLittle();
    } else {
        return demarshalIntBig();
    }
}

uint32_t Demarshaling::demarshal_uint32_t() {
    if( m_priv->m_endian == Endianess::Little ) {
        return static_cast<uint32_t>( demarshalIntLittle() );
    } else {
        return static_cast<uint32_t>( demarshalIntBig() );
    }
}

int64_t Demarshaling::demarshal_int64_t() {
    if( m_priv->m_endian == Endianess::Little ) {
        return demarshalLongLittle();
    } else {
        return demarshalLongBig();
    }
}

uint64_t Demarshaling::demarshal_uint64_t() {
    if( m_priv->m_endian == Endianess::Little ) {
        return static_cast<uint64_t>( demarshalLongLittle() );
    } else {
        return static_cast<uint64_t>( demarshalLongBig() );
    }
}

double Demarshaling::demarshal_double() {
    double ret;
    int64_t val;

    if( m_priv->m_endian == Endianess::Little ) {
        val = demarshalLongLittle();
    } else {
        val = demarshalLongBig();
    }

    memcpy( &ret, &val, sizeof( int64_t ) );

    return ret;
}

std::string Demarshaling::demarshal_string() {
    uint32_t len = demarshal_uint32_t();
    is_valid( len + 1 );
    const char* start = reinterpret_cast<const char*>( m_priv->m_data + m_priv->m_dataPos );
    std::string ret = std::string( start, len );

    m_priv->m_dataPos += len + 1;

    return ret;
}

DBus::Path Demarshaling::demarshal_path() {
    std::string strPath = demarshal_string();
    Path ret = Path( strPath );

    return ret;
}

DBus::Signature Demarshaling::demarshal_signature() {
    uint8_t len = demarshal_uint8_t();
    const char* start = reinterpret_cast<const char*>( m_priv->m_data + m_priv->m_dataPos );
    std::string asStr = std::string( start, len );

    m_priv->m_dataPos += len + 1;

    return Signature( asStr );
}

DBus::Variant Demarshaling::demarshal_variant() {
    DBus::Signature sig = demarshal_signature();
    DBus::SignatureIterator iter = sig.begin();

    switch( iter.type() ) {
    case DataType::BYTE:
        return DBus::Variant( demarshal_uint8_t() );

    case DataType::BOOLEAN:
        return DBus::Variant( demarshal_boolean() );

    case DataType::INT16:
        return DBus::Variant( demarshal_int16_t() );

    case DataType::UINT16:
        return DBus::Variant( demarshal_uint16_t() );

    case DataType::INT32:
        return DBus::Variant( demarshal_int32_t() );

    case DataType::UINT32:
        return DBus::Variant( demarshal_uint32_t() );

    case DataType::INT64:
        return DBus::Variant( demarshal_int64_t() );

    case DataType::UINT64:
        return DBus::Variant( demarshal_uint64_t() );

    case DataType::DOUBLE:
        return DBus::Variant( demarshal_double() );

    case DataType::STRING:
        return DBus::Variant( demarshal_string() );

    case DataType::OBJECT_PATH:
        return DBus::Variant( demarshal_path() );

    case DataType::SIGNATURE:
        return DBus::Variant( demarshal_signature() );

    case DataType::ARRAY:

    //return DBus::Variant( demarshal_uint32_t() );
    case DataType::VARIANT:
        return DBus::Variant( demarshal_variant() );

    case DataType::STRUCT:

    //return "std::tuple";
    case DataType::DICT_ENTRY:

    //return "std::map";
    case DataType::UNIX_FD:

    //return "DBus::FileDescriptor";
    case DataType::INVALID:
        break;
    }

    return DBus::Variant();
}

int16_t Demarshaling::demarshalShortBig() {
    int16_t ret = 0;
    align( 2 );
    is_valid( 2 );

    ret = ( ( m_priv->m_data[ m_priv->m_dataPos ] & 0xFF ) << 8 ) |
        ( ( m_priv->m_data[ m_priv->m_dataPos + 1 ] & 0xFF ) << 0 );


    m_priv->m_dataPos += 2;

    return ret;
}

int16_t Demarshaling::demarshalShortLittle() {
    int16_t ret = 0;
    align( 2 );
    is_valid( 2 );

    ret = ( ( m_priv->m_data[ m_priv->m_dataPos ] & 0xFF ) << 0 ) |
        ( ( m_priv->m_data[ m_priv->m_dataPos + 1 ] & 0xFF ) << 8 );

    m_priv->m_dataPos += 2;

    return ret;
}

int32_t Demarshaling::demarshalIntBig() {
    int32_t ret = 0;
    align( 4 );
    is_valid( 4 );

    ret = static_cast<int32_t>( m_priv->m_data[ m_priv->m_dataPos ] ) << 24  |
        static_cast<int32_t>( m_priv->m_data[ m_priv->m_dataPos + 1 ] ) << 16 |
        static_cast<int32_t>( m_priv->m_data[ m_priv->m_dataPos + 2 ] ) << 8 |
        static_cast<int32_t>( m_priv->m_data[ m_priv->m_dataPos + 3 ] ) << 0 ;

    m_priv->m_dataPos += 4;

    return ret;
}

int32_t Demarshaling::demarshalIntLittle() {
    int32_t ret = 0;
    align( 4 );
    is_valid( 4 );

    ret = static_cast<int32_t>( m_priv->m_data[ m_priv->m_dataPos ] ) << 0  |
        static_cast<int32_t>( m_priv->m_data[ m_priv->m_dataPos + 1 ] ) << 8 |
        static_cast<int32_t>( m_priv->m_data[ m_priv->m_dataPos + 2 ] ) << 16 |
        static_cast<int32_t>( m_priv->m_data[ m_priv->m_dataPos + 3 ] ) << 24 ;

    m_priv->m_dataPos += 4;

    return ret;
}

int64_t Demarshaling::demarshalLongBig() {
    int64_t ret = 0;
    align( 8 );
    is_valid( 8 );

    ret = static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos ] ) << 56 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 1 ] ) << 48 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 2 ] ) << 40 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 3 ] ) << 32 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 4 ] ) << 24 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 5 ] ) << 16 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 6 ] ) << 8 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 7 ] ) << 0 ;

    m_priv->m_dataPos += 8;

    return ret;
}

int64_t Demarshaling::demarshalLongLittle() {
    int64_t ret = 0;
    align( 8 );
    is_valid( 8 );

    ret = static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos ] ) << 0 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 1 ] ) << 8 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 2 ] ) << 16 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 3 ] ) << 24 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 4 ] ) << 32 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 5 ] ) << 40 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 6 ] ) << 48 |
        static_cast<int64_t>( m_priv->m_data[ m_priv->m_dataPos + 7 ] ) << 56 ;

    m_priv->m_dataPos += 8;

    return ret;
}

void Demarshaling::is_valid( uint32_t bytesWanted ) {
    assert( m_priv->m_data != nullptr );
    assert( ( m_priv->m_dataPos + bytesWanted ) <= m_priv->m_dataLen );
}

void Demarshaling::align( int alignment ) {
    if( alignment == 0 ){
        return;
    }
    int bytesToAlign = alignment - ( m_priv->m_dataPos % alignment );

    if( bytesToAlign == alignment ) {
        // already aligned!
        return;
    }

    m_priv->m_dataPos += bytesToAlign;
}

uint32_t Demarshaling::current_offset() const {
    return m_priv->m_dataPos;
}

void Demarshaling::set_endianess( Endianess endian ) {
    m_priv->m_endian = endian;
}

void Demarshaling::set_data_offset( uint32_t offset ) {
    m_priv->m_dataPos = offset;
}
