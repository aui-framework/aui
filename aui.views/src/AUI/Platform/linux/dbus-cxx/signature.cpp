// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009 by Rick L. Vinyard, Jr.                            *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "signature.h"

#include <vector>
#include <stack>
#include <exception>

#include "dbus-cxx-private.h"

#include "types.h"

static const char* LOGGER_NAME = "DBus.Signature";

namespace DBus {

const Signature::size_type npos = std::string::npos;

class Signature::priv_data {
public:
    priv_data() :
        m_valid( false )
    {}

    std::string m_signature;

    std::shared_ptr<priv::SignatureNode> m_startingNode;
    bool m_valid;
};

Signature::Signature() :
    m_priv( std::make_shared<priv_data>() ) {
}

Signature::Signature( const std::string& s, size_type pos, size_type n ):
    m_priv( std::make_shared<priv_data>() ) {
    m_priv->m_signature = std::string( s, pos, n );
    initialize();
}

Signature::Signature( const char* s ) :
    m_priv( std::make_shared<priv_data>() ) {
    m_priv->m_signature = std::string( s );
    initialize();
}

Signature::Signature( const char* s, size_type n ):
    m_priv( std::make_shared<priv_data>() ) {
    m_priv->m_signature = std::string( s, n );
    initialize();
}

Signature::Signature( size_type n, char c ):
    m_priv( std::make_shared<priv_data>() ) {
    m_priv->m_signature = std::string( n, c );
    initialize();
}

Signature::~Signature() {
}

Signature::operator const std::string& () const {
    return m_priv->m_signature;
}

const std::string& Signature::str() const {
    return m_priv->m_signature;
}

Signature& Signature::operator =( const std::string& s ) {
    m_priv->m_signature = s;
    return *this;
}

Signature& Signature::operator =( const char* s ) {
    m_priv->m_signature = s;
    return *this;
}

Signature::iterator Signature::begin() {
    if( !m_priv->m_valid ) { return SignatureIterator(); }

    return SignatureIterator( m_priv->m_startingNode );
}

Signature::const_iterator Signature::begin() const {
    if( !m_priv->m_valid ) { return SignatureIterator(); }

    return SignatureIterator( m_priv->m_startingNode );
}

Signature::iterator Signature::end() {
    return SignatureIterator( nullptr );
}

Signature::const_iterator Signature::end() const {
    return SignatureIterator( nullptr );
}

bool Signature::is_valid() const {
    return m_priv->m_valid;
}

bool Signature::is_singleton() const {
    return m_priv->m_valid &&
        m_priv->m_startingNode != nullptr &&
        m_priv->m_startingNode->m_dataType != DataType::INVALID  &&
        m_priv->m_startingNode->m_next == nullptr;
}

Signature::SignatureNodePointer Signature::create_signature_tree(
    std::string::const_iterator& itr,
    std::stack<ContainerType>* container_stack,
    bool& ok) {
    SignatureNodePointer first;
    SignatureNodePointer current;

    if( container_stack->size() > 64 ) {
        ok = false;
        return nullptr;
    }

    if( itr == m_priv->m_signature.cend() ) {
        return nullptr;
    }

    do {
        DataType data_type = char_to_dbus_type( *itr );
        bool ending_container = is_ending_container( *itr );

        if( data_type == DataType::INVALID ) {
            ok = false;
            return nullptr;
        }

        if( ending_container ) {
            if( container_stack->size() == 0 ) {
                ok = false;
                return nullptr;
            }

            ContainerType currentTop = container_stack->top();

            if( currentTop == ContainerType::STRUCT &&
                data_type == DataType::STRUCT ) {
                return first;
            } else if( currentTop == ContainerType::DICT_ENTRY &&
                data_type == DataType::DICT_ENTRY ) {
                return first;
            } else if (currentTop == ContainerType::ARRAY) {
                return first;
            } else {
                ok = false;
                return nullptr;
            }
        }

        SignatureNodePointer newnode =
            std::make_shared<priv::SignatureNode>( data_type );

        if( current != nullptr ) {
            current->m_next = newnode;
            current = newnode;
        }

        if( first == nullptr ) {
            first = newnode;
            current = newnode;
        }

        TypeInfo ti( data_type );

        if( !container_stack->empty() &&
            container_stack->top() == ContainerType::ARRAY &&
            ti.is_basic() ) {
            // We just added this basic element to the array
            itr++;
            break;
        }

        if( ti.is_container() &&
            data_type != DataType::VARIANT ) {
            ContainerType toPush = char_to_container_type( *itr );
            container_stack->push( toPush );
            itr++;
            current->m_sub = create_signature_tree( itr, container_stack, ok );

            // Check for unbalanced containers
            if( container_stack->top() != toPush ) {
                ok = false;
                return nullptr;
            }

            // Handle array (no end of array character)
            if( toPush == ContainerType::ARRAY &&
                current->m_sub != nullptr ) {
                // Note: need to be special about popping and advancing iterator
                // Assume we have 'aaid' as our signature.  When popping the array
                // off of our stack, we only need to advance the iterator once.
                // So basically, don't advance the iterator unless we're all the way
                // at the begining of the array.
                // This also means that we will either continue or break, depending on
                // if we are at the end of the signature or not, so we know if we need
                // to go on at all.
                bool isArrayEnd = true;

                container_stack->pop();

                if( !container_stack->empty() &&
                    container_stack->top() == ContainerType::ARRAY ) {
                    isArrayEnd = false;
                }

                if( isArrayEnd && itr != m_priv->m_signature.cend() ) {
                    continue;
                }

                break;
            }

            // Check for missing end of container char
            if( itr == m_priv->m_signature.cend() ) {
                ok = false;
                return nullptr;
            }

            // If we're the ending character of a container,
            // advance the iterator so we go to the next character
            ending_container = is_ending_container( *itr );

            if( (ending_container) &&
                (toPush == ContainerType::STRUCT) ) {
                container_stack->pop();
            }
            else if( (ending_container) &&
                     (toPush == ContainerType::DICT_ENTRY) ) {
                container_stack->pop();
                itr++;
                return first;
            } else {
                ok = false;
                return nullptr;
            }
        }

        if( itr != m_priv->m_signature.cend() ) {
            itr++;
        }
    }
    while (itr != m_priv->m_signature.cend() );

    return first;
}

void Signature::print_tree( std::ostream* stream ) const {
    SignatureNodePointer current = m_priv->m_startingNode;

    while( current != nullptr ) {
        *stream << current->m_dataType;
        current = current->m_next;

        if( current == nullptr ) {
            *stream << " (null) ";
        } else {
            *stream << " --> ";
        }
    }
}

void Signature::print_node( std::ostream* stream, priv::SignatureNode* node, int spaces ) const {
    if( node == nullptr ) {
        return;
    }

    //if( node->)
    for( int x = 0; x < spaces; x++ ) {
        *stream << " ";
    }

    *stream << node->m_dataType;
}

void Signature::initialize() {
    m_priv->m_valid = true;
    std::stack<ContainerType> containerStack;
    std::string::const_iterator it = m_priv->m_signature.begin();

    m_priv->m_startingNode = create_signature_tree( it, &containerStack, m_priv->m_valid );

    if( !containerStack.empty() ||
        it != m_priv->m_signature.end() ) {
        SIMPLELOGGER_DEBUG( LOGGER_NAME, "Either stack not empty or signature not used up completely" );
        m_priv->m_valid = false;
    }

    std::ostringstream logmsg;
    logmsg << "Signature \'" << m_priv->m_signature << "\' is ";

    if( m_priv->m_valid ) {
        logmsg << "valid";
    } else {
        logmsg << "invalid";
    }

    SIMPLELOGGER_TRACE( LOGGER_NAME, logmsg.str() );
}

}
