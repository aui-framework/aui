// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <string>
#include <vector>

#ifndef DBUSCXX_PATH_H
#define DBUSCXX_PATH_H

namespace DBus {

/**
 * Represents a DBus Path.  Paths have slashes in them(e.g. /com/example/MusicPlayer1)
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class Path: public std::string {
public:
    Path();

    Path( const std::string& s, size_type pos = 0, size_type n = npos );

    Path( const char* );

    Path( const char* s, size_type n );

    Path( size_type n, char c );

    template<class InputIterator>
    Path( InputIterator first, InputIterator last ): std::string( first, last ) { }

    ~Path();

    operator bool() const;

    bool is_valid() const;

    bool is_root() const;

    /**
     * Decomposes the string into its component elements
     *
     * If the path is invalid or non-existant an empty vector will be
     * returned.
     *
     * If the path is the root path a vector containing a single empty
     * string will be returned. In this way a valid root path is
     * differentiated from an invalid or empty path.
     */
    std::vector<std::string> decomposed() const;

    /**
     * Adds the element string to the path ensuring delimeters are properly
     * taken care of.
     *
     * An element that contains invalid characters will not be added to the
     * path.
     *
     * @return True if the element was succesfully added, false otherwise.
     */
    bool append_element( const std::string& element );

    /**
     * Create a relative path between the two paths.
     * The second param must be below the first, e.g. if first is "/first" and second is "/second" this returns a null path.
     *
     * @param first
     * @param second
     * @return
     */
    static Path relative_path( const Path& first, const Path& second );

};

}

#endif
