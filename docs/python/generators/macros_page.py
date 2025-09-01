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
            output += "\n".join([i[1] for i in common.parse_doxygen(i.doc) if i[0] == "@brief"])
            try:
                examples_lists = examples_page.examples_lists
            except Exception:
                examples_lists = {}

            def _examples_for_name(name: str):
                """Return matches as a list of tuples (category, example, src_path, snippet).

                snippet contains up to 2 lines before and after the matched line.
                """
                found = []
                pat = re.compile(r"\\b" + re.escape(name) + r"\\b")
                for cat, examples in examples_lists.items():
                    for ex in examples:
                        for src in ex.get('srcs', []):
                            try:
                                text = src.read_text(encoding='utf-8', errors='ignore')
                            except Exception:
                                continue
                            m = pat.search(text)
                            if not m:
                                continue
                            # Find line index of match
                            lines = text.splitlines()
                            cum = 0
                            line_idx = 0
                            pos = m.start()
                            for i, l in enumerate(lines):
                                if pos <= cum + len(l):
                                    line_idx = i
                                    break
                                cum += len(l) + 1

                            start = max(0, line_idx - 2)
                            end = min(len(lines), line_idx + 3)
                            snippet = '\n'.join(lines[start:end])
                            found.append((cat, ex, Path(src), snippet))
                return found

            matches = _examples_for_name(i.name)
            if matches:
                output += "\n\n## Examples\n\n"
                for cat, ex, src, snippet in matches:
                    try:
                        src_rel = src.relative_to(Path.cwd())
                    except Exception:
                        src_rel = src
                    extension = common.determine_extension(src)
                    # Render a note block with the file path as title, link to example and snippet
                    output += f"\n??? note \"{src_rel}\"\n\n"
                    output += f"    [{ex['title']}]({ex['id']}.md) — {ex.get('description','')}\n\n"
                    output += f"    ```{extension}\n"
                    for line in snippet.splitlines():
                        output += f"    {line}\n"
                    output += f"    ```\n"
        return output
