// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009 by Rick L. Vinyard, Jr.                            *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "returnmessage.h"
#include "message.h"
#include "callmessage.h"

namespace DBus {

ReturnMessage::ReturnMessage( ) :
    Message() {
}

std::shared_ptr<ReturnMessage> ReturnMessage::create() {
    return std::shared_ptr<ReturnMessage>( new ReturnMessage() );
}

std::shared_ptr<ReturnMessage> ReturnMessage::create( std::shared_ptr<const CallMessage> callee ) {
    std::shared_ptr<ReturnMessage> ret = std::shared_ptr<ReturnMessage>( new ReturnMessage() );
    ret->set_reply_serial( callee->serial() );
    return ret;
}

bool ReturnMessage::set_reply_serial( uint32_t s ) {
    Variant serialField = Variant( s );
    set_header_field( MessageHeaderFields::Reply_Serial, serialField );
    return false;
}

uint32_t ReturnMessage::reply_serial() const {
    Variant field = header_field( MessageHeaderFields::Reply_Serial );

    if( field.type() == DataType::UINT32 ) {
        return field.to_uint32();
    }

    return 0;
}

MessageType ReturnMessage::type() const {
    return MessageType::RETURN;
}

}

