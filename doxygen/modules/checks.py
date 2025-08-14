#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
from pathlib import Path

from modules import regexes, common


# AUI-specific checks here.

def is_valid_workdir():
    if not Path('README.md').exists():
        print(
            "Error: README.md does not exist in the current working directory. Are you running from proper working dir?")
        exit(-1)

class SourceLine:
    file: Path
    line_number: int

    def __init__(self, file: Path, line_number: int):
        self.file = file
        self.line_number = line_number


def are_all_in_group(directory: Path, group: str):
    expected = f"@ingroup {group}"
    ignore_tag = f"// ignore {group}"
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".h"):
                full_path = Path(root) / file

                def read(path: Path):
                    with open(path, 'r') as fis:
                        for line_number, line in enumerate(fis.readlines()):
                            if "// defined here" in line:
                                m = regexes.INCLUDE.match(line)
                                if not m:
                                    common.report_error(path, "bad \"// defined here\" usage", line=line_number)
                                    continue
                                for s in read(common.find_source_root(path) / m.group(1)):
                                    yield s
                                continue

                            yield SourceLine(path, line_number), line

                contents = [i for i in read(full_path)]

                if any([ignore_tag in line[1] for line in contents]):
                    continue

                if not any(["@ingroup" in line[1] for line in contents]):
                    common.report_error(full_path,
                                        "no @ingroup statement in docs; either document with @ingroup or mark the include "
                                        "where the property is defined with \"// defined here\"")

                for src, line in contents:
                    if m := regexes.INGROUP.match(line):
                        matching = m.group(1)
                        if matching != expected:
                            common.report_error(src.file, f"\"{matching}\" does not match \"{expected}\"",
                                                line=src.line_number)

def declarative_notation():
    for root, dirs, files in os.walk("aui.views/src/AUI/View"):
        for file in files:
            if file.endswith(".h"):
                full_path = (Path(root) / file)
                content = full_path.read_text()
                if not "namespace declarative " in content:
                    continue
                if not "@declarativeformof" in content:
                    common.report_error(full_path, "does not contain @declarativeformof")
