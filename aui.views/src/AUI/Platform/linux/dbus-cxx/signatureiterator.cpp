// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009 by Rick L. Vinyard, Jr.                            *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "signatureiterator.h"
#include <cstring>
#include <algorithm>
#include <iterator>
#include "enums.h"
#include "types.h"

namespace DBus {

class SignatureIterator::priv_data {
public:
    priv_data() :
        m_valid( false )
    {}

    priv_data( std::shared_ptr<priv::SignatureNode> startnode ) :
        m_valid( startnode != nullptr ),
        m_current( startnode ),
        m_first( startnode )
    {}

    bool m_valid;
    std::shared_ptr<priv::SignatureNode> m_current;
    std::shared_ptr<priv::SignatureNode> m_first;
};

SignatureIterator::SignatureIterator():
    m_priv( std::make_unique<priv_data>() ) {
}

SignatureIterator::SignatureIterator( const SignatureIterator& other ) :
    m_priv( std::make_unique<priv_data>() ) {
    *m_priv = *other.m_priv;
}

SignatureIterator::SignatureIterator( std::shared_ptr<priv::SignatureNode> startnode ) :
    m_priv( std::make_unique<priv_data>( startnode ) ) {
}

SignatureIterator::~SignatureIterator() {}

void SignatureIterator::invalidate() {
    m_priv->m_valid = false;
}

bool SignatureIterator::is_valid() const {
    return ( m_priv->m_valid && this->type() != DataType::INVALID );
}

SignatureIterator::operator bool() const {
    return this->is_valid();
}

bool SignatureIterator::next() {
    if( !this->is_valid() ) { return false; }

    if( m_priv->m_current->m_next == nullptr ) {
        m_priv->m_current = nullptr;
        m_priv->m_valid = false;
        return false;
    }

    m_priv->m_current = m_priv->m_current->m_next;

    return true;
}

SignatureIterator& SignatureIterator::operator ++() {
    this->next();
    return ( *this );
}

SignatureIterator SignatureIterator::operator ++( int ) {
    SignatureIterator temp_copy;
    temp_copy = *this;
    ++( *this );
    return temp_copy;
}

bool SignatureIterator::operator==( const SignatureIterator& other ) {
    return m_priv->m_current == other.m_priv->m_current;
}

DataType SignatureIterator::type() const {
    if( !m_priv->m_valid ) { return DataType::INVALID; }

    return m_priv->m_current->m_dataType;
}

DataType SignatureIterator::element_type() const {
    if( this->type() != DataType::ARRAY ) { return DataType::INVALID; }

    SignatureIterator subit( m_priv->m_current->m_sub );
    return subit.type();
}

bool SignatureIterator::is_basic() const {
    TypeInfo t( type() );
    return t.is_basic();
}

bool SignatureIterator::is_fixed() const {
    TypeInfo t( type() );
    return t.is_fixed();
}

bool SignatureIterator::is_container() const {
    TypeInfo t( type() );
    return t.is_container();
}

bool SignatureIterator::is_array() const {
    return this->type() == DataType::ARRAY;
}

bool SignatureIterator::is_dict() const {
    return this->is_array() && this->element_type() == DataType::DICT_ENTRY;
}

SignatureIterator SignatureIterator::recurse() {
    std::string substr;

    if( !this->is_container() ) { return SignatureIterator(); }

    SignatureIterator subiter = SignatureIterator( m_priv->m_current->m_sub );

    return subiter;
}

std::string SignatureIterator::signature() const {
    return iterate_over(m_priv->m_first);
}

std::string SignatureIterator::iterate_over(
    std::shared_ptr<priv::SignatureNode> start) const {
    std::string signature;

    for( std::shared_ptr<priv::SignatureNode> current = start;
        current != nullptr;
        current = current->m_next ) {
        if ( current->m_dataType == DataType::STRUCT ) {
            signature += "(" + iterate_over(current->m_sub) + ")";
        } else if ( current->m_dataType == DataType::DICT_ENTRY ) {
            signature += "{" + iterate_over(current->m_sub) + "}";
        } else {
            char dbus_char =
                TypeInfo(current->m_dataType).to_dbus_char();

            signature += dbus_char;

            if ( current->m_dataType == DataType::ARRAY ) {
                signature += iterate_over(current->m_sub);
            }
        }
    }

    return signature;
}

SignatureIterator& SignatureIterator::operator=( const SignatureIterator& other ) {
    if( this != &other ) {
        m_priv->m_valid = other.m_priv->m_valid;
        m_priv->m_current = other.m_priv->m_current;
        m_priv->m_first = other.m_priv->m_first;
    }

    return *this;
}

bool SignatureIterator::has_next() const {
    return m_priv->m_current->m_next != nullptr;
}

}

