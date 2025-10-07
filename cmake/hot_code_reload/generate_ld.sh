#!/usr/bin/env sh
#
# AUI Framework - Declarative UI toolkit for modern C++20
# Copyright (C) 2020-2025 Alex2772 and Contributors
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Usage: ./generate_ld.sh /path/to/elf

ELF="$1"
LDSCRIPT="$2"


echo 'SECTIONS
{
' > "$LDSCRIPT"

nm -n "$ELF" | awk '/ [TVWt] / {
    printf("  .%s 0x%s :\n  {\n    *(.text.%s)\n  }\n\n", $3, $1, $3)
}' >> "$LDSCRIPT"

printf '}
INSERT BEFORE .text;
' >> "$LDSCRIPT"
