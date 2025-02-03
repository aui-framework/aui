#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2024 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

import enum
import glob
from pathlib import Path


class Mode(enum.Enum):
    INSERT_BEFORE = enum.auto()
    INSERT_AFTER = enum.auto()
    DELETE_LINE = enum.auto()
    REPLACE = enum.auto()

class AnchorError(RuntimeError):
    pass

def patch(target: str = None, matcher = None, mode: Mode = None, value = None, unique = False):
    args = {**locals()}
    PREFIX = 'doxygen/out/html'

    patch_path = Path('doxygen/patches/') / target
    if issubclass(type(value), Path):
        value = value.read_bytes().decode("utf-8")
    elif value is None and mode in [Mode.INSERT_AFTER, Mode.INSERT_BEFORE]:
        value = patch_path.read_bytes().decode("utf-8")

    if "*" in target:
        args.pop('target')
        # wildcard
        for file in glob.iglob(f"doxygen/out/html/{target}"):
            try:
                patch(file[len(PREFIX)+1:], **args)
            except AnchorError:
                pass

        return

    target_path = Path(PREFIX) / target
    with open(target_path, 'r') as fis:
        contents = fis.readlines()

    if type(matcher) is str:
        matcher_str = matcher
        matcher = lambda x: matcher_str in x


    def process():
        found = False
        for line in contents:
            if found and unique:
                yield line
                continue
            matcher_result = matcher(line)
            if matcher_result:
                found = True
                if type(matcher_result) is str:
                    yield matcher_result
                elif mode == Mode.INSERT_BEFORE:
                    yield value
                    yield line
                elif mode == Mode.INSERT_AFTER:
                    yield line
                    yield value
                elif mode == Mode.REPLACE:
                    yield line.replace(matcher_str, value)
                elif mode == Mode.DELETE_LINE:
                    pass
            else:
                yield line

        if not found:
            raise AnchorError(f"{target_path} does not contain anchor")

    with open(target_path, 'w') as out:
        for i in process():
            out.write(i)

