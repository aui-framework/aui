// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUS_CXX_SIMPLETRANSPORT_H
#define DBUS_CXX_SIMPLETRANSPORT_H

#include <memory>
#include <vector>
#include <stdint.h>
#include "transport.h"

namespace DBus {

class Message;

namespace priv {

/**
 * The SimpleTransport handles reading and writing over a Unix FD.
 */
class SimpleTransport : public Transport {
private:
    SimpleTransport( int fd, bool initialize );

public:
    ~SimpleTransport();

    /**
     * Create a SimpleTransport.  If the transport is unable to be
     * created, returns an invalid shared_ptr.
     *
     * @param fd The already-open file descriptor
     * @param initialize True if this transport should initialize the
     * connection by sending a single NULL byte
     * @return
     */
    static std::shared_ptr<SimpleTransport> create( int fd, bool initialize );

    ssize_t writeMessage( std::shared_ptr<const Message> message, uint32_t serial );

    std::shared_ptr<Message> readMessage();

    /**
     * Check if this transport is OK
     * @return
     */
    bool is_valid() const;

    int fd() const;

private:
    void purgeData();

private:
    class priv_data;

    std::unique_ptr<priv_data> m_priv;
};

} /* namespace priv */

} /* namespace DBus */

#endif
