// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_VARIANTAPPENDITERATOR_H
#define DBUSCXX_VARIANTAPPENDITERATOR_H

#include <vector>
#include <string>
#include <AUI/Platform/linux/dbus-cxx/signature.h>
#include <AUI/Platform/linux/dbus-cxx/path.h>
#include <AUI/Platform/linux/dbus-cxx/filedescriptor.h>

namespace DBus {

class Variant;

namespace priv {

/**
 * Used to append a single, complete value to a Variant.
 *
 * This is essentially a variant-specific version of MessageAppendIterator
 */
class VariantAppendIterator {
private:
    VariantAppendIterator( Variant* variant, ContainerType t );
public:
    VariantAppendIterator( Variant* variant );
    ~VariantAppendIterator();

    VariantAppendIterator& operator<<( const bool& v );
    VariantAppendIterator& operator<<( const uint8_t& v );
    VariantAppendIterator& operator<<( const int16_t& v );
    VariantAppendIterator& operator<<( const uint16_t& v );
    VariantAppendIterator& operator<<( const int32_t& v );
    VariantAppendIterator& operator<<( const uint32_t& v );
    VariantAppendIterator& operator<<( const int64_t& v );
    VariantAppendIterator& operator<<( const uint64_t& v );
    VariantAppendIterator& operator<<( const double& v );
    VariantAppendIterator& operator<<( const char* v );
    VariantAppendIterator& operator<<( const std::string& v );
    VariantAppendIterator& operator<<( const Signature& v );
    VariantAppendIterator& operator<<( const Path& v );
    VariantAppendIterator& operator<<( const Variant& v );

    template <typename T>
    VariantAppendIterator& operator<<( const std::vector<T>& v ) {
        open_container( ContainerType::ARRAY, DBus::signature( v ) );
        VariantAppendIterator* sub = sub_iterator();

        for( T t : v ) {
            ( *sub ) << t;
        }

        close_container();

        return *this;
    }

    template <typename Key, typename Data>
    VariantAppendIterator& operator<<( const std::map<Key, Data>& dictionary ) {
        std::string sig = signature_dict_data( dictionary );
        typename std::map<Key, Data>::const_iterator it;
        this->open_container( ContainerType::ARRAY, sig );

        for( it = dictionary.begin(); it != dictionary.end(); it++ ) {
            sub_iterator()->open_container( ContainerType::DICT_ENTRY, std::string() );
            *( sub_iterator()->sub_iterator() ) << it->first;
            *( sub_iterator()->sub_iterator() ) << it->second;
            sub_iterator()->close_container();
        }

        this->close_container();

        return *this;
    }

    template <typename... T>
    VariantAppendIterator& operator<<( const std::tuple<T...>& tup ) {
        bool success;
        std::string signature = DBus::signature( tup );
        success = this->open_container( ContainerType::STRUCT, signature.c_str() );
        VariantAppendIterator* subiter = sub_iterator();
        std::apply( [subiter]( auto&& ...arg ) mutable {
            ( *subiter << ... << arg );
        },
        tup );
        this->close_container();

        return *this;
    }

private:
    bool open_container( ContainerType t, const std::string& contained_signature );

    bool close_container( );

    VariantAppendIterator* sub_iterator();

private:
    class priv_data;

    std::shared_ptr<priv_data> m_priv;
};

} /* namespace priv */

} /* namespace DBus */

#endif /* DBUSCXX_VARIANTAPPENDITERATOR_H */
