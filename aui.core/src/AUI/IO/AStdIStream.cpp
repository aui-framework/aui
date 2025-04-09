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

#include "AStdIStream.h"

AStdIStream::StreamBuf::StreamBuf(_<IInputStream> is) : mIs(std::move(is)) {
    setg(0, 0, 0);
    setp(mBuffer, mBuffer + std::size(mBuffer));
}

AStdIStream::StreamBuf::~StreamBuf() {

}

int AStdIStream::StreamBuf::underflow() {
    auto r = mIs->read(mBuffer, std::size(mBuffer));
    if (r == 0) return std::basic_ios<char>::traits_type::eof();
    setg(mBuffer, mBuffer, mBuffer + r);
    return std::basic_ios<char>::traits_type::to_int_type(*gptr());
}

AStdIStream::AStdIStream(_<IInputStream> is) : std::istream(new StreamBuf(std::move(is))) {

}
