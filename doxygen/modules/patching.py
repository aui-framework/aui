#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

from concurrent.futures import ThreadPoolExecutor, as_completed
from enum import Enum, auto
from functools import lru_cache
from glob import iglob
from pathlib import Path


class Mode(Enum):
    INSERT_BEFORE = auto()
    INSERT_AFTER = auto()
    DELETE_LINE = auto()
    REPLACE = auto()


class AnchorError(RuntimeError):
    pass


# 1. Cache patch file contents
@lru_cache(maxsize=None)
def load_patch_snippet(target: str) -> str:
    patch_path = Path("doxygen/patches") / target
    return patch_path.read_text(encoding="utf-8")


def patch(
    target: str,
    matcher=None,
    mode: Mode = None,
    value: str = None,
    unique: bool = False,
):
    PREFIX = Path("doxygen/out/html")

    # 2. Handle wildcard targets in parallel
    if "*" in target:
        pattern = str(PREFIX / target)
        futures = []
        results = []
        with ThreadPoolExecutor() as pool:
            for file_path in iglob(pattern):
                rel_target = Path(file_path).relative_to(PREFIX).as_posix()
                futures.append(
                    pool.submit(patch, rel_target, matcher, mode, value, unique)
                )
            for f in as_completed(futures):
                try:
                    results.append(f.result())
                except AnchorError:
                    pass
        return results

    target_path = PREFIX / target
    if not target_path.exists():
        raise FileNotFoundError(f"{target_path} not found")

    # 3. Determine insertion value
    if value is None and mode in (Mode.INSERT_BEFORE, Mode.INSERT_AFTER):
        value = load_patch_snippet(target)

    # 4. Prepare matcher
    if isinstance(matcher, str):
        needle = matcher

        def matcher_fn(line, **_):
            return needle in line

    else:
        matcher_fn = matcher

    if isinstance(value, Path):
        value = value.read_text(encoding="utf-8")
    # 5. Read once
    original = target_path.read_text(encoding="utf-8").splitlines(keepends=True)
    output = []
    found = False

    # 6. Process lines
    for line in original:
        if found and unique:
            output.append(line)
            continue

        if matcher_fn(line, target_path=target_path):
            found = True

            if isinstance(matcher_fn(line), str):
                output.append(matcher_fn(line))
            elif mode == Mode.INSERT_BEFORE:
                output.extend([value, line])
            elif mode == Mode.INSERT_AFTER:
                output.extend([line, value])
            elif mode == Mode.REPLACE:
                output.append(line.replace(needle, value))
            elif mode == Mode.DELETE_LINE:
                # skip line
                continue
        else:
            output.append(line)

    if not found:
        raise AnchorError(f"No anchor in {target_path}")

    # 7. Write back only if changed
    if output != original:
        target_path.write_text("".join(output), encoding="utf-8")

    return True
