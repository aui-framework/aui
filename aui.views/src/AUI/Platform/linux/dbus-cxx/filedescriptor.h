// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *   Copyright (C) 2014- by Robert Middleton                               *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/

#ifndef DBUS_CXX_FILEDESCRIPTOR
#define DBUS_CXX_FILEDESCRIPTOR

#include <memory>

namespace DBus {

/**
 * A FileDescriptor holds a UNIX file descriptor that can be passed between processes.
 *
 */
class FileDescriptor {
protected:
    FileDescriptor() :
        m_valid( false ),
        m_fd( -1 ) {}

    explicit FileDescriptor( int fd ) :
        m_valid( true ),
        m_fd( fd ) {}

    explicit FileDescriptor( const FileDescriptor& other ) :
        m_valid( other.m_valid ),
        m_fd( other.m_fd ) {}

public:
    static std::shared_ptr<FileDescriptor> create( int fd ) {
        std::shared_ptr<FileDescriptor> p( new FileDescriptor( fd ) );
        return p;
    }

    ~FileDescriptor() {}

    int descriptor() const {
        return m_fd;
    }

    operator bool() const {
        return m_valid;
    }

private:
    bool m_valid;
    int m_fd;
};

}

#endif
