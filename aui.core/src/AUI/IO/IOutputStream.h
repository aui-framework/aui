/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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