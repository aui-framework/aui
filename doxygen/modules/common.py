#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2024 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

from pathlib import Path

error_flag = False

def report_error(file: Path, error_string: str, line: int = None):
    global error_flag
    error_flag = True
    line_string = ""
    if line is not None:
        line_string = f":{line}"
    print(f"{file.absolute()}{line_string}: {error_string}")


def find_source_root(file: Path) -> Path:
    """
    Searches source root.
    :param path: some file in src/ directory, i.e., aui.views/src/AUI/Views.h
    :return: source root, i.e., aui.views/src
    """
    if file.name == "src":
        return file
    return find_source_root(file.parent)

