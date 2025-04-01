#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import subprocess

from modules.config import CONFIG


def doxygen_is_interesting_error(line):
    for i in CONFIG["doxygen_annoyances"]:
        if i in line:
            return False
    return True


def doxygen_parse_stderr(stderr):
    it = iter(stderr.decode('utf-8').split('\n'))

    current_error_lines = []

    for i in it:
        if i.startswith("/") or i.startswith("<unknown"):  # indicates a start of error
            if current_error_lines:
                if doxygen_is_interesting_error(current_error_lines[0]):
                    yield "\n".join(current_error_lines)
                current_error_lines = []

        current_error_lines.append(i)

    if current_error_lines:
        if doxygen_is_interesting_error(current_error_lines[0]):
            yield "\n".join(current_error_lines)
            current_error_lines = []

def invoke():
    print("Doxygen version:", subprocess.run("doxygen -v", shell=True, capture_output=True).stdout.decode('utf-8'))
    result = subprocess.run("doxygen doxygen/Doxyfile", shell=True, capture_output=True)
    doxygen_errors = doxygen_parse_stderr(result.stderr)

    count = 0
    for i in doxygen_errors:
        count += 1
        print(i)
    if count > 0:
        print(f"Error: doxygen failed: {count} error(s).")
        global error_flag
        error_flag = True

