#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import logging
from pathlib import Path

from docs.python.generators import regexes, common


def _process_code_begin(iter):
    for _, line in iter:
        if "AUI_DOCS_CODE_END" in line:
            return
        yield line

log = logging.getLogger('mkdocs')

def parse_tests(path: Path):
    output = ""
    lines = iter(enumerate(path.read_text().split('\n')))
    for line_number, line in lines:
        if match := regexes.TESTCASE_HEADER_H1.match(line):
            log.warning(f'In tests file {path} found usage of HEADER_H1; please use HEADER_H2 instead.')
            output += "## Broken group; see logs"

        if match := regexes.TESTCASE_HEADER_H2.match(line):
            output += "\n"
            output += "## "
            output += match.group(3).replace("_", " ")
            output += " {#"
            output += f'{match.group(2)}_{match.group(3)}'
            output += "}"
            output += "\n"
            continue

        if match := regexes.TESTCASE_HEADER_H3.match(line):
            output += "\n"
            output += "### "
            output += match.group(3).replace("_", " ")
            output += " {#"
            output += f'{match.group(2)}_{match.group(3)}'
            output += "}"
            output += "\n"
            continue

        if comment := regexes.COMMENT.match(line):
            comment_contents = comment.group(1)
            if "AUI_DOCS_CODE_BEGIN" in comment_contents:
                output += "```cpp"
                output += "\n"
                code = [i for i in _process_code_begin(lines)]
                for line in common.strip_indentation(code):
                    line = line.rstrip("\n")
                    if line.endswith("// HIDE"):
                        continue
                    output += line
                    output += "\n"
                output += "```\n"
                continue
            output += comment_contents
            output += "\n"
    return output
