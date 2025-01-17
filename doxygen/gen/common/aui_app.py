#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2024 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import re


def list_all_vars_containing(varname):
    REGEX = re.compile(f'([A-Z_]*{varname}[A-Z_]*)')
    with open('cmake/aui.build.cmake', 'r') as fis:
        vars = sorted(set(REGEX.findall(fis.read())))
    if not vars:
        return

    print(f"@ref docs/aui_app.md populates the following {varname}-related variables (if undefined):")
    for occurrence in vars:
        print(f"- `{occurrence}`")

def list_all_vars_affected_by(varname):
    REGEX = re.compile(f'set\((C[A-Za-z_0-9]+) (.*{varname}.*)\)')
    with open('cmake/aui.build.cmake', 'r') as fis:
        def find():
            for l in fis.readlines():
                if m := REGEX.search(l):
                    yield f"- `{m.group(1)}` <i> = `{m.group(2)}` </i>"
        vars = sorted(set([i for i in find()]))
    if not vars:
        return

    print(f"`{varname}` affects the following CMake variables (if undefined):")
    for occurrence in vars:
        print(occurrence)
