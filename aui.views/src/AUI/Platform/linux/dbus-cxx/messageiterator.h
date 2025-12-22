// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2009,2010 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <stdint.h>
#include <AUI/Platform/linux/dbus-cxx/demangle.h>
#include <AUI/Platform/linux/dbus-cxx/types.h>
#include <AUI/Platform/linux/dbus-cxx/variant.h>
#include <AUI/Platform/linux/dbus-cxx/demarshaling.h>
#include <AUI/Platform/linux/dbus-cxx/signatureiterator.h>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>
#include "enums.h"
#include "error.h"
#include "headerlog.h"
#include "signature.h"

#ifndef DBUSCXX_MESSAGEITERATOR_H
#define DBUSCXX_MESSAGEITERATOR_H

#if defined(__GNUC__) && (__GNUC__ <= 8)
    /*
    * There seems to be a bug with G++ on versions <= 8 where the operator Variant()
    * is trying to call the constructor instead of actually calling the operator on
    * the iterator.  This macro works around this bug
    */
    #define DBUSCXX_MESSAGEITERATOR_OPERATOR_VARIANT(iter) iter.operator DBus::Variant()
    #define DBUSCXX_MESSAGEITERATOR_OPERATOR_SIGNATURE(iter) iter.operator DBus::Signature()
    #define DBUSCXX_MESSAGEITERATOR_OPERATOR_MULTIPLE_RETURN(iter) iter.operator DBus::MultipleReturn()
#else
    #define DBUSCXX_MESSAGEITERATOR_OPERATOR_VARIANT(iter) (DBus::Variant)iter
    #define DBUSCXX_MESSAGEITERATOR_OPERATOR_SIGNATURE(iter) (DBus::Signature)iter
    #define DBUSCXX_MESSAGEITERATOR_OPERATOR_MULTIPLE_RETURN(iter) (DBus::MultipleReturn)iter
#endif

namespace DBus {

class FileDescriptor;
class Message;
template<typename ... T> class MultipleReturn;

/**
 * Extraction iterator allowing values to be retrieved from a message
 *
 * @ingroup message
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class MessageIterator {

private:
    /**
     * Create a new sub-iterator
     *
     * @param d The data type we are iterating over
     * @param sig The signature within the data type
     * @param message Our parent message
     * @param demarshal The demarshaller
     */
    MessageIterator( DataType d,
        SignatureIterator sig,
        const Message* message,
        std::shared_ptr<Demarshaling> demarshal );

public:

    MessageIterator();

    MessageIterator( const Message& message );

    MessageIterator( std::shared_ptr<Message> message );

    /**
     * Returns a pointer to the message associated with this iterator or NULL
     * if no message is associated.
     */
    const Message* message() const;

    /** Invalidates the iterator */
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

    MessageIterator& operator ++();

    MessageIterator operator ++( int );

    bool operator==( const MessageIterator& other );

    /** Returns the argument type that the iterator points to */
    DataType arg_type() const;

    /**
     * Returns the element type of the array that the iterator points to
     *
     * If the iterator does not point to an array TYPE_INVALID is returned.
     */
    DataType element_type() const;

    /** True if the element type is a fixed type */
    bool is_fixed() const;

    /** True if the iterator points to a container */
    bool is_container() const;

    /** True if the iterator points to an array */
    bool is_array() const;

    /** True if the iterator points to a dictionary */
    bool is_dict() const;

    /**
     * If the iterator points to a container recurses into the container returning a sub-iterator.
     *
     * If the iterator does not point to a container returns an empty (invalid) iterator.
     */
    MessageIterator recurse();

    /** Returns the current signature of the iterator */
    std::string signature() const;

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
    operator std::shared_ptr<FileDescriptor>();
    operator Variant();

    template <typename T>
    operator std::vector<T>() {
        if( !this->is_array() ) {
            throw ErrorInvalidTypecast( "MessageIterator: Extracting non array into std::vector" );
        }
        std::vector<T> ret;
        get_array<T>(ret);

        return ret;
    }

    template <typename Key, typename Data>
    operator std::map<Key, Data>() {
        if( !this->is_dict() ) {
            throw ErrorInvalidTypecast( "MessageIterator: Extracting non dict into std::map" );
        }

        return get_dict<Key, Data>();
    }

    template <typename... T>
    operator std::tuple<T...>() {
        std::tuple<T...> tup;
        get_struct<T...>( tup );
        return tup;
    }

    template <typename... T>
    operator DBus::MultipleReturn<T...>() {
        DBus::MultipleReturn<T...> multi_ret;
        get_multiplereturn<T...>( multi_ret );
        return multi_ret;
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
    std::shared_ptr<FileDescriptor> get_filedescriptor();
    Variant get_variant();
    Signature get_signature();

    /**
     * Get values in an array, pushing them back one at a time
     */
    template <typename T>
    void get_array( std::vector<T>& array ) {
        if( !this->is_array() ) { /* Should never happen */
            throw ErrorInvalidTypecast( "MessageIterator: Extracting non array into std::vector" );
        }

        array.clear();

        MessageIterator subiter = this->recurse();

        while( subiter.is_valid() ) {
            //NOTE: we don't need to do subiter.next() here, because
            //operator>> does that for us
            T val {};
            subiter >> val;
            array.push_back( val );
        }
    }

    template <typename... T>
    void get_struct( std::tuple<T...>& tup ) {
        MessageIterator subiter = this->recurse();
        std::apply( [subiter]( auto&& ...arg ) mutable {
            ( subiter >> ... >> arg );
        },
        tup );
    }

    template <typename Key, typename Data>
    void get_dict( std::map<Key, Data>& dict ) {
        MessageIterator subiter = this->recurse();

        while( subiter.is_valid() ) {
            MessageIterator subSubiter = subiter.recurse();

            // When we recurse the second time, our demarshaling gets aligned.
            // This may cause the subiter to then become invalid because of
            // the new byte offset, so we need a second is_valid() check here.
            if( !subiter.is_valid() ){
                break;
            }

            while( subSubiter.is_valid() ) {
                Key val_key;
                Data val_data;

                subSubiter >> val_key;
                subSubiter >> val_data;
                dict[ val_key ] = val_data;
                subSubiter.next();
            }

            subiter.next();
        }
    }

    template <typename... T>
    void get_multiplereturn(MultipleReturn<T...> &v) {
        std::tuple<T...> tup;
        std::apply( [this]( auto&& ...arg ) mutable {
            ( *this >> ... >> arg );
        },
        tup );

        v.m_data = tup;
    }

    template <typename Key, typename Data>
    std::map<Key, Data> get_dict() {
        std::map<Key, Data> newMap;
        get_dict( newMap );
        return newMap;
    }

    template <typename Key, typename Data>
    MessageIterator& operator>>( std::map<Key, Data>& m ) {
        if( !this->is_dict() ) {
            throw ErrorInvalidTypecast( "MessageIterator: Extracting non dict into std::map" );
        }

        get_dict<Key, Data>( m );
        this->next();
        return *this;
    }

    template <typename... T>
    MessageIterator& operator>>( std::tuple<T...>& v ) {
        this->get_struct<T...>( v );
        this->next();
        return *this;
    }

    template <typename T>
    MessageIterator& operator>>( std::vector<T>& v ) {
        if( !this->is_array() ) {
            throw ErrorInvalidTypecast( "MessageIterator: Extracting non array into std::vector" );
        }

        this->get_array<T>( v );
        this->next();
        return *this;
    }

    MessageIterator& operator>>( Variant& v ) {
        v = this->get_variant();
        this->next();
        return *this;
    }

    template<typename ... T>
    MessageIterator& operator>>( MultipleReturn<T...>& v ) {
        this->get_multiplereturn<T...>(v);
        this->next();
        return *this;
    }

    template <typename T>
    MessageIterator& operator>>( T& v ) {
        v = static_cast<T>( *this );
        this->next();
        return *this;
    }

    template<typename T>
    T get(){
        T ret = (T)(*this);
        this->next();
        return ret;
    }


private:
    SignatureIterator signature_iterator();

    /**
     * Align our memory to the specified location.  This skips bytes.
     * This is for internal use only; don't call it in client code!
     *
     * @param alignment
     */
    void align( int alignment );

private:
    class priv_data;

    std::shared_ptr<priv_data> m_priv;

    friend class Variant;
};

}

#endif
