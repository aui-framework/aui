// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "pendingcall.h"
#include "error.h"
#include "message.h"

namespace DBus {


//  PendingCall::PendingCall() :
//      m_canceled( false )
//  {
//  }

//  std::shared_ptr<PendingCall> PendingCall::create()
//  {
//    return std::shared_ptr<PendingCall>( new PendingCall() );
//  }

//  PendingCall::~PendingCall()
//  {
//  }

//  void PendingCall::cancel()
//  {
//      m_canceled = true;
//  }

//  bool PendingCall::completed() const
//  {
//    return m_reply->is_valid();
//  }

//  std::shared_ptr<Message> PendingCall::reply() const
//  {
//    return m_reply;
//  }

//  void PendingCall::block() const
//  {
//      if( m_canceled ){
//          return;
//      }
//  }

//  sigc::signal<void()> PendingCall::signal_notify() const
//  {
//    return m_signal_notify;
//  }

//  void PendingCall::set_reply( std::shared_ptr<Message> msg ){
//      m_reply = msg;
//      m_signal_notify.emit();
//  }

}
