#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
from docs.python.generators import cpp_parser, common
from docs.python.generators.cpp_parser import CppMacro


def define_env(env):
    @env.macro
    def list_macros():
        output = ""
        macros = sorted([i for i in cpp_parser.index if isinstance(i, CppMacro)], key=lambda x: x.name)
        for i in macros:
            output += f"\n\n---\n\n"
            output += f"`{i.name}`\n\n"
            output += "\n".join([i[1] for i in common.parse_doxygen(i.doc) if i[0] == "@brief"])
        return output
