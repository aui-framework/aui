// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009,2010 by Rick L. Vinyard, Jr.             *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/enums.h>
#include <AUI/Platform/linux/dbus-cxx/path.h>
#include <AUI/Platform/linux/dbus-cxx/signature.h>
#include <AUI/Platform/linux/dbus-cxx/filedescriptor.h>
#include <stdint.h>
#include <string>
#include <tuple>
#include <vector>

#ifndef DBUSCXX_TYPES_H
#define DBUSCXX_TYPES_H

namespace DBus {

typedef std::map<DBus::Path,std::map<std::string,std::map<std::string,DBus::Variant>>> ObjectManagerObjects;

class Variant;

inline int typeToDBusType( DataType t ) {
    return static_cast<int>( t );
}

inline int typeToDBusContainerType( ContainerType t ) {
    return static_cast<int>( t );
}

inline DataType char_to_dbus_type( char c ) {
    switch( c ) {
    case 'y':
        return DataType::BYTE;

    case 'b':
        return DataType::BOOLEAN;

    case 'n':
        return DataType::INT16;

    case 'q':
        return DataType::UINT16;

    case 'i':
        return DataType::INT32;

    case 'u':
        return DataType::UINT32;

    case 'x':
        return DataType::INT64;

    case 't':
        return DataType::UINT64;

    case 'd':
        return DataType::DOUBLE;

    case 's':
        return DataType::STRING;

    case 'o':
        return DataType::OBJECT_PATH;

    case 'g':
        return DataType::SIGNATURE;

    case 'a':
        return DataType::ARRAY;

    case 'r':
    case '(':
    case ')':
        return DataType::STRUCT;

    case 'v':
        return DataType::VARIANT;

    case 'e':
    case '{':
    case '}':
        return DataType::DICT_ENTRY;

    case 'h':
        return DataType::UNIX_FD;

    }

    return DataType::INVALID;
}

inline ContainerType char_to_container_type( char c ) {
    switch( c ) {
    case 'a':
        return ContainerType::ARRAY;

    case 'e':
    case '{':
        return ContainerType::DICT_ENTRY;

    case '(':
    case 'r':
        return ContainerType::STRUCT;

    case 'v':
        return ContainerType::VARIANT;
    }

    return ContainerType::None;
}

inline bool is_ending_container( char c ) {
    switch( c ) {
    case '}':
    case ')':
        return true;

    default:
        return false;
    }
}

inline ContainerType char_to_ending_container( char c ) {
    switch( c ) {
    case '}':
        return ContainerType::DICT_ENTRY;

    case ')':
        return ContainerType::STRUCT;
    }

    return ContainerType::None;
}

inline DataType type( const uint8_t& )            { return DataType::BYTE; }
inline DataType type( const bool& )               { return DataType::BOOLEAN; }
inline DataType type( const int16_t& )            { return DataType::INT16; }
inline DataType type( const uint16_t& )           { return DataType::UINT16; }
inline DataType type( const int32_t& )            { return DataType::INT32; }
inline DataType type( const uint32_t& )           { return DataType::UINT32; }
inline DataType type( const int64_t& )            { return DataType::INT64; }
inline DataType type( const uint64_t& )           { return DataType::UINT64; }
inline DataType type( const double& )             { return DataType::DOUBLE; }
inline DataType type( const std::string& ) { return DataType::STRING; }
inline DataType type( const char* )        { return DataType::STRING; }
inline DataType type( const Path& )               { return DataType::OBJECT_PATH; }
inline DataType type( const Signature& )          { return DataType::SIGNATURE; }
template <typename... args>
inline DataType type( const DBus::Variant& )            { return DataType::VARIANT; }
inline DataType type( const FileDescriptor& )     { return DataType::UNIX_FD; }

inline DataType type( const char& )               { return DataType::BYTE; }
inline DataType type( const int8_t& )             { return DataType::BYTE; }

inline DataType type( const float& )               { return DataType::DOUBLE; }

template <typename T>
inline DataType type( const std::vector<T>& ) { return DataType::ARRAY; }

template <typename ...T>
inline DataType type( const std::tuple<T...>& ) { return DataType::STRUCT; }

inline
DataType checked_type_cast( int n ) {
    return ( DataType )( n );
}

/**
 * Contains useful data about the type in order for
 * code generation to happen in an easy manner
 */
class TypeInfo {
public:
    /**
     * The type to get info for.
     */
    TypeInfo( DataType d );

    /**
     * Returns true if this type is templated.
     */
    bool isTemplated() const;

    /**
     * Returns the needed includes in order to use this type.
     */
    std::vector<std::string> includeFilesForType() const;

    /**
     * Returns the C++ type of this DataType.
     */
    std::string cppType() const;

    /** True if the element type is a basic type */
    bool is_basic() const;

    /** True if the element type is a fixed type */
    bool is_fixed() const;

    /** True if the element type is a container */
    bool is_container() const;

    /**
     * Return the alignment of the type
     * @return
     */
    int32_t alignment() const;

    /**
     * Turn this type into a DBus char.
     * @return
     */
    char to_dbus_char() const;

private:
    DataType m_type;
};

std::ostream& operator<<( std::ostream& os, DataType d );

}

#endif
