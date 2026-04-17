// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_VARIANT_ITERATOR_H
#define DBUSCXX_VARIANT_ITERATOR_H

#include <string>
#include <memory>
#include <AUI/Platform/linux/dbus-cxx/types.h>
#include <AUI/Platform/linux/dbus-cxx/error.h>
#include <AUI/Platform/linux/dbus-cxx/signatureiterator.h>

namespace DBus {

class Variant;
class Demarshaling;

namespace priv {

class VariantIterator {
private:
    /**
     * Create a new sub-iterator
     *
     * @param d The data type we are iterating over
     * @param sig The signature within the data type
     * @param variant Our parent variant
     * @param demarshal The demarshaller
     */
    VariantIterator( DataType d,
        SignatureIterator sig,
        const Variant* variant,
        std::shared_ptr<Demarshaling> demarshal );

public:

    VariantIterator();

    VariantIterator( const Variant* variant );

    VariantIterator& operator>>( Variant& v );

    template <typename T>
    VariantIterator& operator>>( T& v ) {
        v = static_cast<T>( *this );
        this->next();
        return *this;
    }

    operator bool();
    operator uint8_t();
    operator uint16_t();
    operator int16_t();
    operator uint32_t();
    operator int32_t();
    operator uint64_t();
    operator int64_t();
    operator double();
    operator std::string();
    operator DBus::Variant();

    template <typename T>
    operator std::vector<T>() {
        if( !is_array() ) {
            throw ErrorInvalidTypecast( "VariantIterator: Extracting non array into std::vector" );
        }

        std::vector<T> retval;
        VariantIterator subiter = this->recurse();

        while( subiter.is_valid() ) {
            //NOTE: we don't need to do subiter.next() here, because
            //operator>> does that for us
            T val {};
            subiter >> val;
            retval.push_back( val );
        }

        return retval;
    }

    template <typename Key, typename Data>
    operator std::map<Key, Data>() {
        if( !this->is_dict() ) {
            throw ErrorInvalidTypecast( "VariantIterator: Extracting non dict into std::map" );
        }

        std::map<Key, Data> dict;
        Key val_key {};
        Data val_data {};

        VariantIterator subiter = this->recurse();

        while( subiter.is_valid() ) {
            VariantIterator subSubiter = subiter.recurse();

            while( subSubiter.is_valid() ) {
                subSubiter >> val_key;
                subSubiter >> val_data;
                dict[ val_key ] = val_data;
                subSubiter.next();
            }

            subiter.next();
        }

        return dict;
    }

    template <typename ...T>
    operator std::tuple<T...>() {
        std::tuple<T...> tup;

        VariantIterator subiter =
            (is_struct() ? recurse() : *this);

        std::apply( [subiter]( auto&& ...arg ) mutable {
            ( subiter >> ... >> arg );
        },
        tup );

        return tup;
    }

    bool        get_bool();
    uint8_t     get_uint8();
    uint16_t    get_uint16();
    int16_t     get_int16();
    uint32_t    get_uint32();
    int32_t     get_int32();
    uint64_t    get_uint64();
    int64_t     get_int64();
    double      get_double();
    std::string get_string();
    Variant get_variant();
    Signature get_signature();

    void invalidate();

    /** True if this is a valid iterator */
    bool is_valid() const;

    /** True if there are any more fields left to iterate over */
    bool has_next() const;

    /**
     * Moves the iterator to the next field and invalidates it if it moves beyond the last field.
     * It is an error to call this function more than once in a row without a corresponding call
     * to get_XXX/operator XXX.
     *
     * @return true if the iterator moves forward, false if there is no next field and the iterator
     *         has been invalidated
     */
    bool next();

    /** Returns the argument type that the iterator points to */
    DataType arg_type() const;

    /**
     * Returns the element type of the array that the iterator points to
     *
     * If the iterator does not point to an array TYPE_INVALID is returned.
     */
    DataType element_type() const;

    VariantIterator& operator ++();

    VariantIterator operator ++( int );

    VariantIterator recurse();

    /** True if the element type is a fixed type */
    bool is_fixed() const;

    /** True if the iterator points to a container */
    bool is_container() const;

    /** True if the iterator points to an array */
    bool is_array() const;

    /** True if the iterator points to a dictionary */
    bool is_dict() const;

    /** True if the iterator points to a struct */
    bool is_struct() const;

private:
    class priv_data;

    std::shared_ptr<priv_data> m_priv;
};

} /* namespace priv */

} /* namespace DBus */

#endif /* DBUS_CXX_VARIANT_ITERATOR_H */
