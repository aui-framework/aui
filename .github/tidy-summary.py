#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2024 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

"""

This script is run on CI/CD. It runs cmake build and then parses the build log to count occurrences of each diagnostic
and prints them in descending order. It is handy to identify which diagnostic rules are causing issues and adjust the
build configuration accordingly. It is also useful to track the progress of fixing these issues over time.

"""


import re
import subprocess
import sys
from pathlib import Path

REGEX_DIAGNOSTIC = re.compile(r'.+ \[([a-zA-Z0-9-]+)(,.+)?\]$')
assert REGEX_DIAGNOSTIC.match("/home/AUI/Common/ASmallVector.h:326:5: warning: function 'contains' should be marked [[nodiscard]] [modernize-use-nodiscard,-warnings-as-errors]").group(1) == 'modernize-use-nodiscard'


if __name__ == '__main__':
    if not Path('CMakeCache.txt').is_file():
        raise RuntimeError('This script should be run inside preconfigured CMake build directory.')

    # In .clang-tidy, we've disabled WarningsAsErrors. So, we need to ensure we are building all the files to capture
    # all problems.
    if subprocess.run("cmake --build . -t clean", shell=True).returncode != 0:
        exit(-1)

    cmake_process = subprocess.Popen("cmake --build .", shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    cmake_process.stdout.readline()

    def lines(pipe):
        while True:
            l = pipe.readline()
            if not l:
                return
            l = l.decode('utf-8').rstrip('\n')
            print(l)
            yield l

    count = {}
    for line in lines(cmake_process.stdout):
        line = line.strip()
        if m := REGEX_DIAGNOSTIC.match(line):
            diagnostic_name = m.group(1)
            if diagnostic_name.startswith("-W"):
                # TODO unskip warnings
                continue

            count[diagnostic_name] = count.get(diagnostic_name, 0) + 1

    count_as_list = count.items()
    print('Sorted by count:')
    for i in sorted(count_as_list, key=lambda x: x[1], reverse=True):
        print(f"{i[0]}: {i[1]}")

    print('')
    print('Sorted by name:')
    for i in sorted(count_as_list, key=lambda x: x[0]):
        print(f"{i[0]}: {i[1]}")

    print('')
    total = sum(count.values())
    print(f'Total: {total}')
    if total > 0:
        exit(-1)


