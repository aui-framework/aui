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
        """Collect examples for all macros and return list of rendered note blocks."""
        examples_blocks = []
        try:
            examples_lists = examples_page.examples_lists
        except Exception:
            examples_lists = {}

        macros = sorted([i for i in cpp_parser.index if isinstance(i, CppMacro)], key=lambda x: x.name)

        for i in macros:
            name = i.name
            pat = re.compile(r"\\b" + re.escape(name) + r"\\b")
            for cat, examples in examples_lists.items():
                for ex in examples:
                    for src in ex.get('srcs', []):
                        try:
                            text = src.read_text(encoding='utf-8', errors='ignore')
                        except Exception:
                            continue
                        if not pat.search(text):
                            continue
                        # find first match and build snippet
                        m = pat.search(text)
                        lines = text.splitlines()
                        pos = m.start()
                        cum = 0
                        line_idx = 0
                        for idx, l in enumerate(lines):
                            if pos <= cum + len(l):
                                line_idx = idx
                                break
                            cum += len(l) + 1
                        start = max(0, line_idx - 2)
                        end = min(len(lines), line_idx + 3)
                        snippet = '\n'.join(lines[start:end])
                        try:
                            src_rel = Path(src).relative_to(Path.cwd())
                        except Exception:
                            src_rel = Path(src)
                        extension = common.determine_extension(src)
                        block = ''
                        block += f"??? note \"{src_rel}\"\n"
                        block += f"    [{ex['title']}]({ex['id']}.md) — {ex.get('description','')}\n"
                        block += f"    ```{extension}\n"
                        for line in snippet.splitlines():
                            block += f"    {line}\n"
                        block += f"    ```\n"
                        examples_blocks.append(block)
        return examples_blocks

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
