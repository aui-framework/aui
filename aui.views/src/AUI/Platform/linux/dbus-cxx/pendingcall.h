// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009,2010 by Rick L. Vinyard, Jr.             *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <memory>

#ifndef DBUSCXX_PENDING_CALL_H
#define DBUSCXX_PENDING_CALL_H

namespace DBus {
class Message;

/**
 * Monitors an asynchronous call, emitting a signal when a response is received
 *
 * @ingroup message
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
//  class PendingCall: public sigc::trackable
//  {
//    protected:

//      PendingCall( );

//    public:
//      static std::shared_ptr<PendingCall> create( );

//      virtual ~PendingCall();

//      /**
//       * Cancel the pending call; that is, signal_notify() will not
//       * be emitted if and when the reply eventually comes back.
//       */
//      void cancel();

//      /**
//       * Check to see if the reply has actually come back.
//       */
//      bool completed() const;

//      /**
//       * Check to see if the
//       * @return
//       */
////      bool is_timeout() const;

//      /**
//       * Get the reply that this pending call represents.  If
//       * completed() is not true, or this call has been canceled,
//       * returns an invalid pointer.
//       *
//       * @return
//       */
//      std::shared_ptr<Message> reply() const;

//      void block() const;

//      sigc::signal<void()> signal_notify() const;

//      void set_reply( std::shared_ptr<Message> msg );

//    private:

//      sigc::signal<void()> m_signal_notify;

//      std::shared_ptr<Message> m_reply;

//      bool m_canceled;

//  };

}

#endif
