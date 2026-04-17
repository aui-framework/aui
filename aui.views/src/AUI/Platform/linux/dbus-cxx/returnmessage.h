// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <stdint.h>
#include <AUI/Platform/linux/dbus-cxx/message.h>
#include <memory>
#include <string>

#ifndef DBUSCXX_RETURNMESSAGE_H
#define DBUSCXX_RETURNMESSAGE_H

namespace DBus {

class CallMessage;

/**
 * Represents a DBus call return message
 *
 * This class inherits from Message and uses the dbus_message_* methods to
 * create an interface for a return message that is a response to a call
 * message.
 *
 * @ingroup message
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class ReturnMessage : public Message {
protected:

    ReturnMessage( );

public:
    static std::shared_ptr<ReturnMessage> create();

    static std::shared_ptr<ReturnMessage> create( std::shared_ptr<const CallMessage> callee );

    bool set_reply_serial( uint32_t );

    uint32_t reply_serial() const;

    virtual MessageType type() const;

};


}

#endif
