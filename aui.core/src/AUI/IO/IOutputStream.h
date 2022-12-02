// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <cstring>
#include <AUI/Traits/values.h>
#include "AEOFException.h"


class IOutputStream: public aui::noncopyable
{
public:
	virtual ~IOutputStream() = default;

    /**
     * @brief Writes exact <code>size</code> bytes to stream. Blocking (waiting for write all data) is allowed.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd>An implementation can throw any exception that subclasses <a href="#AIOException">AIOException</a>.</dd>
     * </dl>
     * @param dst source buffer
     * @param size source buffer's size. > 0
     */
	virtual void write(const char* src, size_t size) = 0;

    /**
     * Writes data using AUI serialization (see AUI/Traits/serializable.h)
     * @param t value to write
     */
    template<typename T>
    void write(const T& t);

    /**
     * Writes data using AUI serialization (see AUI/Traits/serializable.h)
     * @param t value to write
     */
    template<typename T>
    IOutputStream& operator<<(const T& t) {
        write<T>(t);
        return *this;
    }
};

#include <AUI/Traits/serializable.h>

template<typename T>
void IOutputStream::write(const T& t) {
    aui::serialize(*this, t);
}