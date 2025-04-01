/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <ostream>

template<typename Iterator>
struct AContainerPrinter {
    Iterator mBegin;
    Iterator mEnd;

    AContainerPrinter(Iterator mBegin, Iterator mEnd) : mBegin(mBegin), mEnd(mEnd) {}

    template<typename Container>
    AContainerPrinter(const Container& c): AContainerPrinter(c.begin(), c.end()) {}
};

template<typename Container>
AContainerPrinter(const Container& c) -> AContainerPrinter<decltype(c.begin())>;

template<typename Iterator>
inline std::ostream& operator<<(std::ostream& o, const AContainerPrinter<Iterator>& printer) noexcept {
    o << '{';
    for (auto it = printer.mBegin; it != printer.mEnd; ++it) {
        if (it != printer.mBegin) {
            o << ", ";
        }
        o << *it;
    }
    o << '}';
    return o;
}