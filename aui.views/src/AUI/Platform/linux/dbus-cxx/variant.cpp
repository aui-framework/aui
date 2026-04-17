// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/variant.h>
#include <AUI/Platform/linux/dbus-cxx/messageiterator.h>
#include <AUI/Platform/linux/dbus-cxx/marshaling.h>
#include <AUI/Platform/linux/dbus-cxx/dbus-cxx-private.h>
#include <AUI/Platform/linux/dbus-cxx/signatureiterator.h>
#include <AUI/Platform/linux/dbus-cxx/validator.h>
#include <stdint.h>
#include <utility>
#include <assert.h>
#include "enums.h"
#include "path.h"
#include "signature.h"
#include "utility.h"

static const char* LOGGER_NAME = "DBus.Variant";

namespace DBus { class FileDescriptor; }

using DBus::Variant;

Variant::Variant():
    m_currentType( DataType::INVALID )
{}

Variant::Variant( uint8_t byte ) :
    m_currentType( DataType::BYTE ),
    m_signature( DBus::signature( byte ) ),
    m_dataAlignment( 1 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( byte );
}

Variant::Variant( bool b ) :
    m_currentType( DataType::BOOLEAN ),
    m_signature( DBus::signature( b ) ),
    m_dataAlignment( 4 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( b );
}

Variant::Variant( int16_t i ) :
    m_currentType( DataType::INT16 ),
    m_signature( DBus::signature( i ) ),
    m_dataAlignment( 2 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( i );
}

Variant::Variant( uint16_t i ):
    m_currentType( DataType::UINT16 ),
    m_signature( DBus::signature( i ) ),
    m_dataAlignment( 2 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( i );
}

Variant::Variant( int32_t i ) :
    m_currentType( DataType::INT32 ),
    m_signature( DBus::signature( i ) ),
    m_dataAlignment( 4 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( i );
}

Variant::Variant( uint32_t i ) :
    m_currentType( DataType::UINT32 ),
    m_signature( DBus::signature( i ) ),
    m_dataAlignment( 4 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( i );
}

Variant::Variant( int64_t i ) :
    m_currentType( DataType::INT64 ),
    m_signature( DBus::signature( i ) ),
    m_dataAlignment( 8 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( i );
}

Variant::Variant( uint64_t i ) :
    m_currentType( DataType::UINT64 ),
    m_signature( DBus::signature( i ) ),
    m_dataAlignment( 8 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( i );
}

Variant::Variant( double i ) :
    m_currentType( DataType::DOUBLE ),
    m_signature( DBus::signature( i ) ),
    m_dataAlignment( 8 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( i );
}

Variant::Variant( const char* cstr ) :
    Variant( std::string( cstr ) ) {}

Variant::Variant( std::string str ) :
    m_currentType( DataType::STRING ),
    m_signature( DBus::signature( str ) ),
    m_dataAlignment( 4 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( str );
}

Variant::Variant( DBus::Signature sig ) :
    m_currentType( DataType::SIGNATURE ),
    m_signature( DBus::signature( sig ) ),
    m_dataAlignment( 1 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( sig );
}

Variant::Variant( DBus::Path path )  :
    m_currentType( DataType::OBJECT_PATH ),
    m_signature( DBus::signature( path ) ),
    m_dataAlignment( 4 ) {
    Marshaling marshal( &m_marshaled, DBus::default_endianess() );
    marshal.marshal( path );
}

Variant::Variant( const Variant& other ) :
    m_currentType( other.m_currentType ),
    m_signature( other.m_signature ),
    m_marshaled( other.m_marshaled ),
    m_dataAlignment( other.m_dataAlignment )
{}

Variant::Variant( Variant&& other ) :
    m_currentType( std::exchange( other.m_currentType, DataType::INVALID ) ),
    m_signature( std::move( other.m_signature ) ),
    m_marshaled( std::move( other.m_marshaled ) ),
    m_dataAlignment( std::exchange( other.m_dataAlignment, 0 ) ){
}

Variant::~Variant() {}

DBus::Signature Variant::signature() const {
    return m_signature;
}

DBus::DataType Variant::type() const {
    return m_currentType;
}

Variant Variant::createFromDemarshal( Signature sig, std::shared_ptr<Demarshaling> demarshal, const std::vector<int>& filedescriptors, uint32_t depth ) {
    Variant v;
    SignatureIterator sigiter = sig.begin();
    DBus::DataType dt = sigiter.type();
    TypeInfo ti( dt );
    Marshaling marshal( &v.m_marshaled, DBus::default_endianess() );

    depth++;
    if( depth > Validator::maximum_message_depth() ) {
        throw ErrorMessageNestedTooDeep();
    }

    v.m_signature = sig;
    v.m_currentType = dt;
    v.m_dataAlignment = ti.alignment();
    demarshal->align( ti.alignment() );

    SIMPLELOGGER_TRACE(LOGGER_NAME, "Creating Variant from signature " << sig << ".  Current offset after alignment: " << demarshal->current_offset() );

    switch( dt ){
    case DataType::BYTE:
        marshal.marshal( demarshal->demarshal_uint8_t() );
        break;

    case  DataType::BOOLEAN:
        marshal.marshal( demarshal->demarshal_boolean() );
        break;

    case  DataType::INT16:
        marshal.marshal( demarshal->demarshal_int16_t() );
        break;

    case  DataType::UINT16:
        marshal.marshal( demarshal->demarshal_uint16_t() );
        break;

    case  DataType::INT32:
        marshal.marshal( demarshal->demarshal_int32_t() );
        break;

    case  DataType::UINT32:
        marshal.marshal( demarshal->demarshal_uint32_t() );
        break;

    case  DataType::INT64:
        marshal.marshal( demarshal->demarshal_int64_t() );
        break;

    case  DataType::UINT64:
        marshal.marshal( demarshal->demarshal_uint64_t() );
        break;

    case  DataType::DOUBLE:
        marshal.marshal( demarshal->demarshal_double() );
        break;

    case  DataType::STRING:
        marshal.marshal( demarshal->demarshal_string() );
        break;

    case  DataType::OBJECT_PATH:
        marshal.marshal( demarshal->demarshal_string() );
        break;

    case  DataType::SIGNATURE:
        marshal.marshal( demarshal->demarshal_signature() );
        break;

    case DBus::DataType::ARRAY:
        v.recurseArray( sigiter.recurse(), demarshal, &marshal, filedescriptors, depth );
        break;

    case DBus::DataType::STRUCT:
        v.recurseStruct( sigiter.recurse(), demarshal, &marshal, filedescriptors, depth );
        break;

    case DBus::DataType::VARIANT:
        // We have a variant inside of a variant.  Whoever creates APIs like this,
        // I really really don't like you.
    {
        Signature sig = demarshal->demarshal_signature();
        Variant v2 = createFromDemarshal(sig, demarshal, filedescriptors, depth );

        v = Variant::createFromVariant(v2);
    }
        break;

    default:
        // Not yet supported
    {
        std::string err_text = "Type ";
        if(dt == DataType::INVALID){
            err_text.append( "(invalid)" );
        }else{
            err_text.push_back( (char)dt );
        }
        err_text.append( " unable to be parsed in variant" );
        throw ErrorUnableToParse(err_text.c_str());
        }
    }

    SIMPLELOGGER_TRACE(LOGGER_NAME, "Created Variant.  Current offset: " << demarshal->current_offset() );

    return v;
}

void Variant::recurseArray( SignatureIterator sigit, std::shared_ptr<Demarshaling> demarshal, Marshaling* marshal, const std::vector<int>& filedescriptors, uint32_t depth ){
    DataType dt = sigit.type();
    TypeInfo ti( dt );
    std::vector<uint8_t> workingData;
    Marshaling workingMarshal( &workingData, DBus::default_endianess() );
    uint32_t array_size = demarshal->demarshal_uint32_t();
    uint32_t current_location = demarshal->current_offset();
    uint32_t ending_offset = current_location + array_size;

    depth++;
    if( depth > Validator::maximum_message_depth() ) {
        throw ErrorMessageNestedTooDeep();
    }

    marshal->marshal( array_size );

    SIMPLELOGGER_TRACE(LOGGER_NAME, "Creating Variant array with signature " << sigit.type() << ".  Current: " << current_location << " ending: " << ending_offset );

    while( current_location < ending_offset ) {
        if(dt == DataType::DICT_ENTRY){
            recurseDictEntry( sigit.recurse(), demarshal, marshal, ending_offset, filedescriptors, depth );
        }else{
            demarshal->align( ti.alignment() );
            marshal->align( ti.alignment() );
            remarshal( dt, sigit, demarshal, marshal, filedescriptors, depth );
        }

        current_location = demarshal->current_offset();
    }
}

void Variant::recurseDictEntry( SignatureIterator iter, std::shared_ptr<Demarshaling> demarshal, Marshaling* marshal, uint32_t ending_offset, const std::vector<int>& filedescriptors, uint32_t depth ){
    DataType key_type = iter.type();
    iter++;
    DataType value_type = iter.type();
    TypeInfo ti( key_type );
    uint32_t current_location = demarshal->current_offset();

    depth++;
    if( depth > Validator::maximum_message_depth() ) {
        throw ErrorMessageNestedTooDeep();
    }

    marshal->align(8);
    demarshal->align(8);

    SIMPLELOGGER_TRACE(LOGGER_NAME, "Creating Variant dict entry with types " << key_type << "," << value_type
                       << ".  Current offset: " << current_location << " ending offset: " << ending_offset );

    while( current_location < ending_offset ) {
        demarshal->align(8);
        marshal->align(8);
        remarshal( key_type, iter, demarshal, marshal, filedescriptors, depth );
        remarshal( value_type, iter, demarshal, marshal, filedescriptors, depth );

        current_location = demarshal->current_offset();
    }
}

DBus::Variant Variant::createFromMessage( MessageIterator ) {
    throw ErrorUnableToParse("Old method, do not call.  Why are you calling this method directly?");
}

void Variant::recurseArray( MessageIterator iter, Marshaling* marshal ){}

void Variant::recurseDictEntry( MessageIterator iter, Marshaling* marshal ){}

void Variant::recurseStruct( MessageIterator iter, Marshaling* marshal ){}

void Variant::recurseStruct( SignatureIterator sigit, std::shared_ptr<Demarshaling> demarshal, Marshaling* marshal, const std::vector<int>& filedescriptors, uint32_t depth ) {
    marshal->align( 8 );
    demarshal->align( 8 );

    depth++;
    if( depth > Validator::maximum_message_depth() ) {
        throw ErrorMessageNestedTooDeep();
    }

    while( sigit.is_valid() ) {
        DataType dt = sigit.type();
        TypeInfo ti( dt );

        remarshal( dt, sigit, demarshal, marshal, filedescriptors, depth );

        sigit++;
    }
}

void Variant::remarshal(DataType dt, SignatureIterator sigit, std::shared_ptr<Demarshaling> demarshal, Marshaling *marshal, const std::vector<int>& filedescriptors, uint32_t depth ){
    switch( dt ) {
    case DataType::BYTE:
        marshal->marshal( demarshal->demarshal_uint8_t() );
        break;

    case  DataType::BOOLEAN:
        marshal->marshal( demarshal->demarshal_boolean() );
        break;

    case  DataType::INT16:
        marshal->marshal( demarshal->demarshal_int16_t() );
        break;

    case  DataType::UINT16:
        marshal->marshal( demarshal->demarshal_uint16_t() );
        break;

    case  DataType::INT32:
        marshal->marshal( demarshal->demarshal_int32_t() );
        break;

    case  DataType::UINT32:
        marshal->marshal( demarshal->demarshal_uint32_t() );
        break;

    case  DataType::INT64:
        marshal->marshal( demarshal->demarshal_int64_t() );
        break;

    case  DataType::UINT64:
        marshal->marshal( demarshal->demarshal_uint64_t() );
        break;

    case  DataType::DOUBLE:
        marshal->marshal( demarshal->demarshal_double() );
        break;

    case  DataType::STRING:
        marshal->marshal( demarshal->demarshal_string() );
        break;

    case  DataType::OBJECT_PATH:
        marshal->marshal( demarshal->demarshal_string() );
        break;

    case  DataType::SIGNATURE:
        marshal->marshal( demarshal->demarshal_signature() );
        break;

    case DBus::DataType::ARRAY:
        recurseArray( sigit.recurse(), demarshal, marshal, filedescriptors, depth );
        break;

    case DBus::DataType::STRUCT:
        recurseStruct( sigit.recurse(), demarshal, marshal, filedescriptors, depth );
        break;

    case DBus::DataType::VARIANT:
    {
        Signature sig = demarshal->demarshal_signature();
        Variant v = createFromDemarshal(sig, demarshal, filedescriptors, depth );
        marshal->marshal(sig);
        TypeInfo ti( v.type() );
        marshal->align( ti.alignment() );


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
                marshal->align( 8 );
            }
            marshal->marshal( (*dataToMarshal)[x] );
        }
    }
        break;

    default:
    {
        std::string err_text = "Type ";
        err_text.push_back( (char)dt );
        err_text.append( " currently unable to be parsed in variant" );
        throw ErrorUnableToParse(err_text.c_str());
        }
    }
}

const std::vector<uint8_t>* Variant::marshaled() const {
    return &m_marshaled;
}

int Variant::data_alignment() const {
    return m_dataAlignment;
}

bool Variant::operator==( const Variant& other ) const {
    bool sameType = other.type() == type();
    bool vectorsEqual = false;

    if( sameType ) {
        vectorsEqual = other.m_marshaled == m_marshaled;
    }

    return sameType && vectorsEqual;
}

Variant& Variant::operator=( const Variant& other ) {
    m_currentType = other.m_currentType;
    m_signature = other.m_signature;
    m_marshaled = other.m_marshaled;
    m_dataAlignment = other.m_dataAlignment;

    return *this;
}

bool Variant::to_bool() const {
    if( m_currentType != DataType::BOOLEAN ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_bool();
}

uint8_t Variant::to_uint8() const {
    if( m_currentType != DataType::BYTE ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_uint8();
}

uint16_t Variant::to_uint16() const {
    if( m_currentType != DataType::UINT16 ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_uint16();
}

int16_t Variant::to_int16() const {
    if( m_currentType != DataType::INT16 ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_int16();
}

uint32_t Variant::to_uint32() const {
    if( m_currentType != DataType::UINT32 ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_uint32();
}

int32_t Variant::to_int32() const {
    if( m_currentType != DataType::INT32 ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_int32();
}

uint64_t Variant::to_uint64() const {
    if( m_currentType != DataType::UINT64 ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_uint64();
}

int64_t Variant::to_int64() const {
    if( m_currentType != DataType::INT64 ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_int64();
}

double Variant::to_double() const {
    if( m_currentType != DataType::DOUBLE ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_double();
}

std::string Variant::to_string() const {
    if( m_currentType != DataType::STRING ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_string();
}

DBus::Path Variant::to_path() const {
    if( m_currentType != DataType::OBJECT_PATH ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_string();
}

DBus::Signature Variant::to_signature() const {
    if( m_currentType != DataType::SIGNATURE ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_signature();
}

Variant Variant::to_variant() const{
    if( m_currentType != DataType::VARIANT ) {
        throw ErrorBadVariantCast();
    }

    priv::VariantIterator vi( this );
    return vi.get_variant();
}

Variant::operator bool(){
    return to_bool();
}

Variant::operator uint8_t(){
    return to_uint8();
}

Variant::operator uint16_t(){
    return to_uint16();
}

Variant::operator int16_t(){
    return to_int16();
}

Variant::operator uint32_t(){
    return to_uint32();
}

Variant::operator int32_t(){
    return to_int32();
}

Variant::operator uint64_t(){
    return to_uint64();
}

Variant::operator int64_t(){
    return to_int64();
}

Variant::operator double(){
    return to_double();
}

Variant::operator std::string(){
    return to_string();
}

Variant::operator DBus::Path(){
    return to_path();
}

Variant::operator DBus::Signature(){
    return to_signature();
}

namespace DBus {

std::ostream& operator<<( std::ostream& os, const Variant& var ) {
    os << "DBus::Variant[" << var.type() << "=";

    switch( var.type() ) {
    case DataType::BYTE:
        os << var.to_uint8();
        break;

    case DataType::BOOLEAN:
        os << var.to_bool();
        break;

    case DataType::INT16:
        os << var.to_int16();
        break;

    case DataType::UINT16:
        os << var.to_uint16();
        break;

    case DataType::INT32:
        os << var.to_int32();
        break;

    case DataType::UINT32:
        os << var.to_uint32();
        break;

    case DataType::INT64:
        os << var.to_int64();
        break;

    case DataType::UINT64:
        os << var.to_uint64();
        break;

    case DataType::DOUBLE:
        os << var.to_double();
        break;

    case DataType::STRING:
        os << var.to_string();
        break;

    case DataType::OBJECT_PATH:
        os << var.to_path();
        break;

    case DataType::SIGNATURE:
        os << var.to_signature();
        break;

    case DataType::ARRAY:
        os << "array";
        break;

    case DataType::VARIANT:
        os << "variant";
        break;

    case DataType::STRUCT:
        os << "struct";
        break;

    case DataType::DICT_ENTRY:
        os << "dict_entry";
        break;

    case DataType::UNIX_FD:
        os << "unix_fd";
        break;

    case DataType::INVALID:
        break;
    }

    os << "]";
    return os;
}

Variant Variant::createFromVariant( Variant& other ){
    Variant v;
    TypeInfo ti(DataType::VARIANT);

    v.m_currentType = DataType::VARIANT;
    v.m_signature = DBus::signature(v);
    v.m_dataAlignment = ti.alignment();

    Marshaling marshal(&v.m_marshaled, DBus::default_endianess());
    marshal.marshal(other);

    return v;
}

}
