#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

import pytest
from pathlib import Path

import doxygen

def test_parse_comment_lines():
    iterator = iter(Path('test_data/AString.h').read_text().splitlines())
    for i in iterator:
        if "/**" in iterator:
            break
    assert doxygen.parse_comment_lines(iterator) =="""@brief Represents a Unicode character string.
@ingroup core
@details
AString stores a string of 16-bit chars, where each char corresponds to one UTF-16 code unit. Unicode characters with
code values above 65535 are stored using two consecutive chars.

Unicode is an international standard that supports most of the writing systems in use today."""

