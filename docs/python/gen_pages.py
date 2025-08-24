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
