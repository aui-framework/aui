#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

from pathlib import Path


def determine_extension(f: Path):
    extension = f.suffix
    if extension == ".h":
        extension = "cpp"
    elif str(f.name) == "CMakeLists.txt":
        extension = "cmake"
    elif str(f.name) == ".clang-format":
        extension = "yaml"

    return extension
