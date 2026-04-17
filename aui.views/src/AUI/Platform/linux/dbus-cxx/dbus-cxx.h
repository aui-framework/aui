// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2009,2010 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_DBUS_H
#define DBUSCXX_DBUS_H

#if !(__cplusplus >= 201703L)
#error "dbus-cxx requires at least C++17.  Check your compiler flags(-std=c++17 for GCC/clang, /Zc:__cplusplus /std:c++17 for MSVC)"
#endif

#include <AUI/Platform/linux/dbus-cxx/callmessage.h>
#include <AUI/Platform/linux/dbus-cxx/connection.h>
#include <AUI/Platform/linux/dbus-cxx/signal.h>
#include <AUI/Platform/linux/dbus-cxx/dispatcher.h>
#include <AUI/Platform/linux/dbus-cxx/enums.h>
#include <AUI/Platform/linux/dbus-cxx/error.h>
#include <AUI/Platform/linux/dbus-cxx/errormessage.h>
#include <AUI/Platform/linux/dbus-cxx/interface.h>
#include <AUI/Platform/linux/dbus-cxx/interfaceproxy.h>
#include <AUI/Platform/linux/dbus-cxx/messageappenditerator.h>
#include <AUI/Platform/linux/dbus-cxx/message.h>
#include <AUI/Platform/linux/dbus-cxx/messageiterator.h>
#include <AUI/Platform/linux/dbus-cxx/methodbase.h>
#include <AUI/Platform/linux/dbus-cxx/methodproxybase.h>
#include <AUI/Platform/linux/dbus-cxx/object.h>
#include <AUI/Platform/linux/dbus-cxx/objectproxy.h>
#include <AUI/Platform/linux/dbus-cxx/pendingcall.h>
#include <AUI/Platform/linux/dbus-cxx/returnmessage.h>
#include <AUI/Platform/linux/dbus-cxx/signalbase.h>
#include <AUI/Platform/linux/dbus-cxx/signalmessage.h>
#include <AUI/Platform/linux/dbus-cxx/signalproxy.h>
#include <AUI/Platform/linux/dbus-cxx/signature.h>
#include <AUI/Platform/linux/dbus-cxx/signatureiterator.h>
#include <AUI/Platform/linux/dbus-cxx/utility.h>
#include <AUI/Platform/linux/dbus-cxx/variant.h>
#include <AUI/Platform/linux/dbus-cxx/filedescriptor.h>
#include <AUI/Platform/linux/dbus-cxx/simplelogger_defs.h>
#include <AUI/Platform/linux/dbus-cxx/standalonedispatcher.h>
#include <AUI/Platform/linux/dbus-cxx/propertyproxy.h>
#include <AUI/Platform/linux/dbus-cxx/property.h>
#include <AUI/Platform/linux/dbus-cxx/multiplereturn.h>

#endif
