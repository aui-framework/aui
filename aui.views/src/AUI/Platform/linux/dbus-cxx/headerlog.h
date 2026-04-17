// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *   Copyright (C) 2014- by Robert Middleton                               *
 *   rm5248@rm5248.com                                                     *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_HEADERLOG_H
#define DBUSCXX_HEADERLOG_H

#include <sstream>
#include <AUI/Platform/linux/dbus-cxx/simplelogger_defs.h>

extern simplelogger_log_function dbuscxx_log_function;

#define DBUSCXX_LOG_CSTR_HEADER( logger, message, level ) do{\
        if( !dbuscxx_log_function ) break;\
        struct SL_LogLocation location;\
        location.line_number = __LINE__;\
        location.file = __FILE__;\
        location.function = __func__;\
        dbuscxx_log_function( logger, &location, level, message );\
    } while(0)

#define DBUSCXX_DEBUG_STDSTR( logger, message ) do{\
        std::stringstream stream;\
        stream << message;\
        DBUSCXX_LOG_CSTR_HEADER( logger, stream.str().c_str(), SL_DEBUG);\
    } while(0)

#endif
