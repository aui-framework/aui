#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
from docs.python.generators import cpp_parser, common, examples_page
from docs.python.generators.cpp_parser import CppMacro
from pathlib import Path
import re
from docs.python.generators.examples_helpers import collect_macro_examples_blocks


def define_env(env):
    @env.macro
    def list_macros():
        output = ""
        macros = sorted([i for i in cpp_parser.index if isinstance(i, CppMacro)], key=lambda x: x.name)
        for i in macros:
            output += f"\n\n---\n\n"
            output += f"`{i.name}`\n\n"
            output += "\n".join([j[1] for j in common.parse_doxygen(i.doc) if j[0] == "@brief"])
        return output

    def _collect_examples_blocks():
        try:
            examples_lists = examples_page.examples_lists
        except Exception:
            examples_lists = {}
        return collect_macro_examples_blocks(examples_lists)

    @env.macro
    def list_macro_examples():
        """Render the Examples section for macros — place this macro at the end of the page."""
        blocks = _collect_examples_blocks()
        if not blocks:
            return ""
        output = "\n\n## Examples\n\n"
        for b in blocks:
            output += f"\n{b}\n"
        return output
