// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <stdint.h>
#include <AUI/Platform/linux/dbus-cxx/path.h>
#include <AUI/Platform/linux/dbus-cxx/signatureiterator.h>
#include <any>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <stack>

#include "enums.h"

#ifndef DBUSCXX_SIGNATURE_H
#define DBUSCXX_SIGNATURE_H

#define DBUSCXX_TYPE_INVALID_AS_STRING "\0"
#define DBUSCXX_TYPE_BYTE_AS_STRING "y"
#define DBUSCXX_TYPE_BOOLEAN_AS_STRING "b"
#define DBUSCXX_TYPE_INT16_AS_STRING "n"
#define DBUSCXX_TYPE_UINT16_AS_STRING "q"
#define DBUSCXX_TYPE_INT32_AS_STRING "i"
#define DBUSCXX_TYPE_UINT32_AS_STRING "u"
#define DBUSCXX_TYPE_INT64_AS_STRING "x"
#define DBUSCXX_TYPE_UINT64_AS_STRING "t"
#define DBUSCXX_TYPE_DOUBLE_AS_STRING "d"
#define DBUSCXX_TYPE_STRING_AS_STRING "s"
#define DBUSCXX_TYPE_SIGNATURE_AS_STRING "g"
#define DBUSCXX_TYPE_OBJECT_PATH_AS_STRING "o"
#define DBUSCXX_TYPE_VARIANT_AS_STRING "v"
#define DBUSCXX_TYPE_UNIX_FD_AS_STRING "h"
#define DBUSCXX_TYPE_ARRAY_AS_STRING "a"
#define DBUSCXX_DICT_ENTRY_BEGIN_CHAR_AS_STRING "{"
#define DBUSCXX_DICT_ENTRY_END_CHAR_AS_STRING "}"
#define DBUSCXX_STRUCT_BEGIN_CHAR_AS_STRING "("
#define DBUSCXX_STRUCT_END_CHAR_AS_STRING ")"

namespace DBus {

namespace priv {
/**
 * Represents a single entry in our graph of the signature
 */
class SignatureNode {
public:
    typedef std::shared_ptr<SignatureNode> SignatureNodePointer;

    SignatureNode(DataType data_type) :
        m_dataType(data_type),
        m_next(),
        m_sub(){}

    DataType m_dataType;
    SignatureNodePointer m_next;
    SignatureNodePointer m_sub;
};

}

class FileDescriptor;
class Variant;
template<typename... T> class MultipleReturn;

/**
 * Represents a DBus signature.  DBus signatures indicate what type of
 * data the message contains/the method parameters.
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class Signature {
public:
    typedef SignatureIterator iterator;

    typedef const SignatureIterator const_iterator;

    typedef std::string::size_type size_type;

    static const size_type npos = std::string::npos;

    Signature();

    Signature( const std::string& s, size_type pos = 0, size_type n = npos );

    Signature( const char* );

    Signature( const char* s, size_type n );

    Signature( size_type n, char c );

    //      template<class InputIterator>
    //      Signature( InputIterator first, InputIterator last ): m_signature( first, last ) { }

    ~Signature();

    operator const std::string& () const;

    const std::string& str() const;

    Signature& operator=( const std::string& s );

    Signature& operator=( const char* s );

    bool operator==( const std::string& s ) const { return str() == s; }

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    /** True if the signature is a valid DBus signature */
    bool is_valid() const;

    /** True if the signature is a valid DBus signature and contains exactly one complete type */
    bool is_singleton() const;

    /**
     * Print the signature tree to the given ostream.
     *
     * @param stream
     */
    void print_tree( std::ostream* stream ) const;

private:
    void initialize();

    typedef priv::SignatureNode::SignatureNodePointer SignatureNodePointer;

    SignatureNodePointer create_signature_tree(
        std::string::const_iterator& itr,
        std::stack<ContainerType>* container_stack,
        bool& ok);

    void print_node(
        std::ostream* stream,
        priv::SignatureNode* node,
        int spaces) const;

private:
    class priv_data;


    std::shared_ptr<priv_data> m_priv;
};

template <typename... T>
inline std::string signature( const std::tuple<T...>& );

inline std::string signature( uint8_t )     { return DBUSCXX_TYPE_BYTE_AS_STRING; }
inline std::string signature( bool )        { return DBUSCXX_TYPE_BOOLEAN_AS_STRING; }
inline std::string signature( int16_t )     { return DBUSCXX_TYPE_INT16_AS_STRING; }
inline std::string signature( uint16_t )    { return DBUSCXX_TYPE_UINT16_AS_STRING; }
inline std::string signature( int32_t )     { return DBUSCXX_TYPE_INT32_AS_STRING; }
inline std::string signature( uint32_t )    { return DBUSCXX_TYPE_UINT32_AS_STRING; }
inline std::string signature( int64_t )     { return DBUSCXX_TYPE_INT64_AS_STRING; }
inline std::string signature( uint64_t )    { return DBUSCXX_TYPE_UINT64_AS_STRING;      }
inline std::string signature( double )      { return DBUSCXX_TYPE_DOUBLE_AS_STRING;      }
inline std::string signature( std::string ) { return DBUSCXX_TYPE_STRING_AS_STRING;      }
inline std::string signature( Signature )   { return DBUSCXX_TYPE_SIGNATURE_AS_STRING;   }
inline std::string signature( Path )        { return DBUSCXX_TYPE_OBJECT_PATH_AS_STRING; }
inline std::string signature( const DBus::Variant& )     { return DBUSCXX_TYPE_VARIANT_AS_STRING; }
inline std::string signature( const std::shared_ptr<FileDescriptor> )  { return DBUSCXX_TYPE_UNIX_FD_AS_STRING; }
template<typename... T>
inline std::string signature( const DBus::MultipleReturn<T...>& )     { return DBUSCXX_TYPE_INVALID_AS_STRING; }


template <typename T> inline std::string signature( const std::vector<T>& ) { T t {}; return DBUSCXX_TYPE_ARRAY_AS_STRING + signature( t ); }

template <typename Key, typename Data> inline std::string signature( const std::map<Key, Data>& ) {
    Key k {}; Data d {};
    std::string sig;
    sig = DBUSCXX_TYPE_ARRAY_AS_STRING;
    sig += DBUSCXX_DICT_ENTRY_BEGIN_CHAR_AS_STRING +
        signature( k ) + signature( d ) +
        DBUSCXX_DICT_ENTRY_END_CHAR_AS_STRING;
    return sig;
}

//Note: we need to have two different signature() methods for dictionaries; this is because
//when introspecting, we need to use the normal signature() so that it comes up properly.
//However, when we are sending out data, that signature would give us an extra array signature,
//which is not good.  Hence, this method is only used when we need to send out a dict
template <typename Key, typename Data> inline std::string signature_dict_data( const std::map<Key, Data>& ) {
    Key k {}; Data d {};
    std::string sig;
    sig = DBUSCXX_DICT_ENTRY_BEGIN_CHAR_AS_STRING +
        signature( k ) + signature( d ) +
        DBUSCXX_DICT_ENTRY_END_CHAR_AS_STRING;
    return sig;
}

template<typename... T_arg>
inline std::string signature_multiple_return_data( const DBus::MultipleReturn<T_arg...>& );

namespace priv {
/*
 * dbus_signature class - signature of a given type
 */
template<typename... argn>
class dbus_signature;

template<> class dbus_signature<> {
public:
    std::string dbus_sig() const {
        return "";
    }
};

template<typename arg1, typename... argn>
class dbus_signature<arg1, argn...> : public dbus_signature<argn...> {
public:
    std::string dbus_sig() const {
        arg1 arg {};
        return signature( arg ) + dbus_signature<argn...>::dbus_sig();
    }
};

} /* namespace priv */

template<typename... T_arg>
inline std::string signature( const std::tuple<T_arg...>& ) {
    priv::dbus_signature<T_arg...> sig;

    return DBUSCXX_STRUCT_BEGIN_CHAR_AS_STRING +
        sig.dbus_sig() +
        DBUSCXX_STRUCT_END_CHAR_AS_STRING;
}

template<typename... T_arg>
inline std::string signature_multiple_return_data( const DBus::MultipleReturn<T_arg...>& ){
    priv::dbus_signature<T_arg...> sig;

    return sig.dbus_sig();
}

inline
std::ostream& operator<<( std::ostream& sout, const DBus::Signature& sig ) {
    sout << "DBus::Signature[" << sig.str() << "]";
    return sout;
}

}

#endif
