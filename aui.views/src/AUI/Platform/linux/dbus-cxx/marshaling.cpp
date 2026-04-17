// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "marshaling.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <AUI/Platform/linux/dbus-cxx/variant.h>
#include <AUI/Platform/linux/dbus-cxx/path.h>
#include <AUI/Platform/linux/dbus-cxx/signature.h>
#include <AUI/Platform/linux/dbus-cxx/signatureiterator.h>

using DBus::Marshaling;

class Marshaling::priv_data {
public:
    priv_data() :
        m_data( nullptr ),
        m_endian( Endianess::Big ) {}

    std::vector<uint8_t>* m_data;
    Endianess m_endian;
};

Marshaling::Marshaling() {
    m_priv = std::make_shared<priv_data>();
}

Marshaling::Marshaling( std::vector<uint8_t>* data, Endianess endian ) {
    m_priv = std::make_shared<priv_data>();
    m_priv->m_data = data;
    m_priv->m_endian = endian;
}

Marshaling::~Marshaling() {
}

void Marshaling::marshal( bool v ) {
    if( m_priv->m_endian == Endianess::Big ) {
        marshalIntBig( v );
    } else {
        marshalIntLittle( v );
    }
}

void Marshaling::marshal( uint8_t v ) {
    m_priv->m_data->push_back( v );
}

void Marshaling::marshal( int16_t v ) {
    if( m_priv->m_endian == Endianess::Big ) {
        marshalShortBig( v );
    } else {
        marshalShortLittle( v );
    }
}

void Marshaling::marshal( uint16_t v ) {
    if( m_priv->m_endian == Endianess::Big ) {
        marshalShortBig( v );
    } else {
        marshalShortLittle( v );
    }
}

void Marshaling::marshal( int32_t v ) {
    if( m_priv->m_endian == Endianess::Big ) {
        marshalIntBig( v );
    } else {
        marshalIntLittle( v );
    }
}

void Marshaling::marshal( uint32_t v ) {
    if( m_priv->m_endian == Endianess::Big ) {
        marshalIntBig( v );
    } else {
        marshalIntLittle( v );
    }
}

void Marshaling::marshal( int64_t v ) {
    if( m_priv->m_endian == Endianess::Big ) {
        marshalLongBig( v );
    } else {
        marshalLongLittle( v );
    }
}

void Marshaling::marshal( uint64_t v ) {
    if( m_priv->m_endian == Endianess::Big ) {
        marshalLongBig( v );
    } else {
        marshalLongLittle( v );
    }
}

void Marshaling::marshal( double v ) {
    uint64_t data;
    std::memcpy( &data, &v, sizeof( uint64_t ) );

    if( m_priv->m_endian == Endianess::Big ) {
        marshalLongBig( data );
    } else {
        marshalLongLittle( data );
    }
}

void Marshaling::marshal( std::string v ) {
    uint32_t len = v.size();
    marshal( len );

    for( const char& c : v ) {
        m_priv->m_data->push_back( c );
    }

    m_priv->m_data->push_back( 0 );
}

void Marshaling::marshal( Path v ) {
    std::string data = std::string( v );
    marshal( data );
}

void Marshaling::marshal( Signature v ) {
    std::string data = v.str();
    m_priv->m_data->push_back( data.size() & 0xFF );

    for( const char& c : data ) {
        m_priv->m_data->push_back( c );
    }

    m_priv->m_data->push_back( 0 );
}

void Marshaling::align( int alignment ) {
    int bytesToAlign = alignment - ( m_priv->m_data->size() % alignment );

    if( bytesToAlign == alignment ) {
        // already aligned!
        return;
    }

    for( int x = 0; x < bytesToAlign; x++ ) {
        m_priv->m_data->push_back( 0 );
    }
}

void Marshaling::marshalShortBig( uint16_t toMarshal ) {
    align( 2 );
    m_priv->m_data->push_back( ( toMarshal & 0xFF00 ) >> 8 );
    m_priv->m_data->push_back( ( toMarshal & 0x00FF ) >> 0 );
}

void Marshaling::marshalIntBig( uint32_t toMarshal ) {
    align( 4 );
    m_priv->m_data->push_back( ( toMarshal & 0xFF000000 ) >> 24 );
    m_priv->m_data->push_back( ( toMarshal & 0x00FF0000 ) >> 16 );
    m_priv->m_data->push_back( ( toMarshal & 0x0000FF00 ) >> 8 );
    m_priv->m_data->push_back( ( toMarshal & 0x000000FF ) >> 0 );
}

void Marshaling::marshalLongBig( uint64_t toMarshal ) {
    align( 8 );
    m_priv->m_data->push_back( ( toMarshal & 0xFF00000000000000 ) >> 56 );
    m_priv->m_data->push_back( ( toMarshal & 0x00FF000000000000 ) >> 48 );
    m_priv->m_data->push_back( ( toMarshal & 0x0000FF0000000000 ) >> 40 );
    m_priv->m_data->push_back( ( toMarshal & 0x000000FF00000000 ) >> 32 );
    m_priv->m_data->push_back( ( toMarshal & 0x00000000FF000000 ) >> 24 );
    m_priv->m_data->push_back( ( toMarshal & 0x0000000000FF0000 ) >> 16 );
    m_priv->m_data->push_back( ( toMarshal & 0x000000000000FF00 ) >> 8 );
    m_priv->m_data->push_back( ( toMarshal & 0x00000000000000FF ) >> 0 );
}

void Marshaling::marshalShortLittle( uint16_t toMarshal ) {
    align( 2 );
    m_priv->m_data->push_back( ( toMarshal & 0x00FF ) >> 0 );
    m_priv->m_data->push_back( ( toMarshal & 0xFF00 ) >> 8 );
}

void Marshaling::marshalIntLittle( uint32_t toMarshal ) {
    align( 4 );
    m_priv->m_data->push_back( ( toMarshal & 0x000000FF ) >> 0 );
    m_priv->m_data->push_back( ( toMarshal & 0x0000FF00 ) >> 8 );
    m_priv->m_data->push_back( ( toMarshal & 0x00FF0000 ) >> 16 );
    m_priv->m_data->push_back( ( toMarshal & 0xFF000000 ) >> 24 );
}

void Marshaling::marshalLongLittle( uint64_t toMarshal ) {
    align( 8 );
    m_priv->m_data->push_back( ( toMarshal & 0x00000000000000FF ) >> 0 );
    m_priv->m_data->push_back( ( toMarshal & 0x000000000000FF00 ) >> 8 );
    m_priv->m_data->push_back( ( toMarshal & 0x0000000000FF0000 ) >> 16 );
    m_priv->m_data->push_back( ( toMarshal & 0x00000000FF000000 ) >> 24 );
    m_priv->m_data->push_back( ( toMarshal & 0x000000FF00000000 ) >> 32 );
    m_priv->m_data->push_back( ( toMarshal & 0x0000FF0000000000 ) >> 40 );
    m_priv->m_data->push_back( ( toMarshal & 0x00FF000000000000 ) >> 48 );
    m_priv->m_data->push_back( ( toMarshal & 0xFF00000000000000 ) >> 56 );
}

void Marshaling::set_data( std::vector<uint8_t>* data ) {
    m_priv->m_data = data;
}

void Marshaling::set_endianess( Endianess endian ) {
    m_priv->m_endian = endian;
}

void Marshaling::marshal( const Variant& v ) {
    Signature signature = v.signature();
    const std::vector<uint8_t>* data = v.marshaled();

    m_priv->m_data->reserve( m_priv->m_data->size()
        + signature.str().size()
        + data->size()
        + 12 /* Extra alignment bytes, if needed */ );

    marshal( signature );
    align(v.data_alignment());

    for( const uint8_t& byte : *data ) {
        m_priv->m_data->push_back( byte );
    }
}

void Marshaling::marshal_at_offset( uint32_t offset, uint32_t value ) {
    if( m_priv->m_endian == Endianess::Little ) {
        ( *m_priv->m_data )[offset++] = ( value & 0x000000FF ) >> 0;
        ( *m_priv->m_data )[offset++] = ( value & 0x0000FF00 ) >> 8;
        ( *m_priv->m_data )[offset++] = ( value & 0x00FF0000 ) >> 16;
        ( *m_priv->m_data )[offset++] = ( value & 0xFF000000 ) >> 24;
    } else {
        ( *m_priv->m_data )[offset++] = ( value & 0xFF000000 ) >> 24;
        ( *m_priv->m_data )[offset++] = ( value & 0x00FF0000 ) >> 16;
        ( *m_priv->m_data )[offset++] = ( value & 0x0000FF00 ) >> 8;
        ( *m_priv->m_data )[offset++] = ( value & 0x000000FF ) >> 0;
    }
}

uint32_t Marshaling::currentOffset() const {
    return m_priv->m_data->size();
}
