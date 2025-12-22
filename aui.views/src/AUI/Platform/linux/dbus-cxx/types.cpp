// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2019 by Robert Middleton.                               *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <AUI/Platform/linux/dbus-cxx/types.h>
#include <AUI/Platform/linux/dbus-cxx/enums.h>
#include <string>
#include <vector>

namespace DBus {

TypeInfo::TypeInfo( DataType d ) : m_type( d ) {}

bool TypeInfo::isTemplated() const {
    switch( m_type ) {
    case DataType::ARRAY:
    case DataType::DICT_ENTRY:
    case DataType::STRUCT:
        return true;

    default:
        break;
    }

    return false;
}

std::vector<std::string> TypeInfo::includeFilesForType() const {
    std::vector<std::string> ret;

    switch( m_type ) {
    case DataType::BYTE:
    case DataType::INT16:
    case DataType::UINT16:
    case DataType::INT32:
    case DataType::UINT32:
    case DataType::INT64:
    case DataType::UINT64:
        ret.push_back( "stdint.h" );
        break;

    case DataType::STRING:
        ret.push_back( "string" );
        break;

    case DataType::ARRAY:
        ret.push_back( "vector" );
        break;

    case DataType::DICT_ENTRY:
        ret.push_back( "map" );
        break;

    case DataType::STRUCT:
        ret.push_back( "tuple" );
        break;

    case DataType::VARIANT:
        ret.push_back( "dbus-cxx/variant.h" );
        break;

    default:
        break;
    }

    return ret;
}

std::string TypeInfo::cppType() const {
    switch( m_type ) {
    case DataType::BYTE:
        return "uint8_t";

    case DataType::BOOLEAN:
        return "bool";

    case DataType::INT16:
        return "int16_t";

    case DataType::UINT16:
        return "uint16_t";

    case DataType::INT32:
        return "int32_t";

    case DataType::UINT32:
        return "uint32_t";

    case DataType::INT64:
        return "int64_t";

    case DataType::UINT64:
        return "uint64_t";

    case DataType::DOUBLE:
        return "double";

    case DataType::STRING:
        return "std::string";

    case DataType::OBJECT_PATH:
        return "DBus::Path";

    case DataType::SIGNATURE:
        return "DBus::Signature";

    case DataType::ARRAY:
        return "std::vector";

    case DataType::VARIANT:
        return "DBus::Variant";

    case DataType::STRUCT:
        return "std::tuple";

    case DataType::DICT_ENTRY:
        return "std::map";

    case DataType::UNIX_FD:
        return "DBus::FileDescriptor";

    case DataType::INVALID:
        break;
    }

    return std::string( "Invalid" );
}

bool TypeInfo::is_basic() const {
    switch( m_type ) {
    case DataType::BYTE:
    case DataType::INT16:
    case DataType::UINT16:
    case DataType::INT32:
    case DataType::UINT32:
    case DataType::INT64:
    case DataType::UINT64:
    case DataType::STRING:
    case DataType::OBJECT_PATH:
    case DataType::SIGNATURE:
    case DataType::UNIX_FD:
        return true;

    default:
        return false;
    }
}

bool TypeInfo::is_fixed() const {
    switch( m_type ) {
    case DataType::BYTE:
    case DataType::INT16:
    case DataType::UINT16:
    case DataType::INT32:
    case DataType::UINT32:
    case DataType::INT64:
    case DataType::UINT64:
        return true;

    default:
        return false;
    }
}

bool TypeInfo::is_container() const {
    switch( m_type ) {
    case DataType::ARRAY:
    case DataType::STRUCT:
    case DataType::VARIANT:
    case DataType::DICT_ENTRY:
        return true;

    default:
        return false;
    }
}

int32_t TypeInfo::alignment() const {
    switch( m_type ) {
    case DataType::BYTE:
    case DataType::SIGNATURE:
    case DataType::VARIANT:
        return 1;

    case DataType::UINT16:
    case DataType::INT16:
        return 2;

    case DataType::BOOLEAN:
    case DataType::INT32:
    case DataType::UINT32:
    case DataType::STRING:
    case DataType::OBJECT_PATH:
    case DataType::ARRAY:
    case DataType::UNIX_FD:
        return 4;

    case DataType::DOUBLE:
    case DataType::INT64:
    case DataType::UINT64:
    case DataType::STRUCT:
    case DataType::DICT_ENTRY:
        return 8;

    default:
        break;
    }

    return 0;
}

char TypeInfo::to_dbus_char() const {
    switch( m_type ) {
    case DataType::BYTE:
        return 'y';

    case DataType::BOOLEAN:
        return 'b';

    case DataType::INT16:
        return 'n';

    case DataType::UINT16:
        return 'q';

    case DataType::INT32:
        return 'i';

    case DataType::UINT32:
        return 'u';

    case DataType::INT64:
        return 'x';

    case DataType::UINT64:
        return 't';

    case DataType::DOUBLE:
        return 'd';

    case DataType::STRING:
        return 's';

    case DataType::OBJECT_PATH:
        return 'o';

    case DataType::SIGNATURE:
        return 'g';

    case DataType::ARRAY:
        return 'a';

    case DataType::VARIANT:
        return 'v';

    case DataType::UNIX_FD:
        return 'h';

    default:
    case DataType::INVALID:
        break;
    }

    return '\0';
}

std::ostream& operator<<( std::ostream& os, DataType d ) {
    switch( d ) {
    case DataType::BYTE:
        os << "DataType::BYTE";
        break;

    case DataType::BOOLEAN:
        os << "DataType::BOOLEAN";
        break;

    case DataType::INT16:
        os << "DataType::INT16";
        break;

    case DataType::UINT16:
        os << "DataType::UINT16";
        break;

    case DataType::INT32:
        os << "DataType::INT32";
        break;

    case DataType::UINT32:
        os << "DataType::UINT32";
        break;

    case DataType::INT64:
        os << "DataType::INT64";
        break;

    case DataType::UINT64:
        os << "DataType::UINT64";
        break;

    case DataType::DOUBLE:
        os << "DataType::DOUBLE";
        break;

    case DataType::STRING:
        os << "DataType::STRING";
        break;

    case DataType::OBJECT_PATH:
        os << "DataType::OBJECT_PATH";
        break;

    case DataType::SIGNATURE:
        os << "DataType::SIGNATURE";
        break;

    case DataType::ARRAY:
        os << "DataType::ARRAY";
        break;

    case DataType::VARIANT:
        os << "DataType::VARIANT";
        break;

    case DataType::STRUCT:
        os << "DataType::STRUCT";
        break;

    case DataType::DICT_ENTRY:
        os << "DataType::DICT_ENTRY";
        break;

    case DataType::UNIX_FD:
        os << "DataType::UNIX_FD";
        break;

    case DataType::INVALID:
        os << "DataType::INVALID";
        break;
    }

    return os;
}


}
