#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import sys
from pathlib import Path

from docs.python.generators import examples_page, doxygen

l = str(Path(__file__).parent)
sys.path.append(l)

import mkdocs_gen_files


examples_page.gen_pages()
doxygen.gen_pages()

# Generate aui.boot.md from the #[==[DOCUMENTATION...]==] block in aui.boot.cmake
_aui_boot_cmake = Path.cwd() / "aui.boot.cmake"
_aui_boot_text = _aui_boot_cmake.read_text()
import re as _re
_m = _re.search(r'#\[==\[DOCUMENTATION\n(.*?)\n\]==\]', _aui_boot_text, _re.DOTALL)
if _m:
    with mkdocs_gen_files.open("aui.boot.md", "w") as _fos:
        _fos.write(_m.group(1))
else:
    raise RuntimeError("Could not find #[==[DOCUMENTATION ... ]==] block in aui.boot.cmake")
