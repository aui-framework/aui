#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import re

def define_env(env):
    @env.macro
    def cmake_list_all_vars_affected_by(file, varname, display_name):
        output = ''
        if not display_name:
            display_name = varname
        def print_vars():
            VAR = re.compile(f'set\\(((CMAKE|CPACK|MACOS)[A-Za-z_0-9]+) (.*{varname}.*)\\)')
            with open(file, 'r') as fis:
                def find():
                    for l in fis.readlines():
                        if m := VAR.search(l):
                            yield m.group(1), m.group(3)
                vars_set = sorted(set([i for i in find()]))
            if not vars_set:
                return vars_set

            nonlocal output
            output += f"`{display_name}` populates the following CMake variables (only if they haven't been defined already):\n\n"
            for occurrence in vars_set:
                output += f"- `{occurrence[0]}`  = `{occurrence[1]}`\n"
            return vars_set

        already_printed_vars_set = print_vars()

        with open(file, 'r') as fis:
            def find():
                REGEX = re.compile("\\$\\{" + varname + "\\}")
                for l in fis.readlines():
                    l = l.strip()
                    if l.startswith("#"):
                        continue
                    if any([i[0] in l for i in already_printed_vars_set]):
                        continue
                    if REGEX.search(l) and "set" in l:
                        yield f"- `{l}`"
            already_printed_vars_set = sorted(set([i for i in find()]))
        if not already_printed_vars_set:
            return output

        output += f"\n\nThe following additional variables and properties are affected:\n\n"
        for occurrence in already_printed_vars_set:
            output += occurrence + "\n"
        return output


    @env.macro
    def cmake_list_all_vars_containing(file, varname):
        REGEX = re.compile(f'\(([A-Za-z_]+)( (.*)\))?( # (.*))?')
        with open(file, 'r') as fis:
            vars_set = [(i[0], i[2], i[4]) for i in REGEX.findall(fis.read()) if varname in f"{i}"]
        if not vars_set:
            return
        vars_set = set(vars_set)
        vars_set = sorted(vars_set)

        output = f"[aui_app] populates the following {varname}-related variables (only if they haven't been defined already):\n\n"
        for var_name, value, comment in vars_set:
            output += f"- `{var_name}`"
            if value:
                output += f" = `{value}`"
            if comment:
                output += f" ({comment})"
            output += '\n'
        return output

