#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2024 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
from pathlib import Path

from modules.config import CONFIG
from bs4 import BeautifulSoup as soup


def run():
    for root, dirs, files in os.walk(CONFIG['output']):
        for file in files:
            if not file.endswith(".html"):
                continue
            full_path = Path(root) / file
            s = soup(full_path.read_bytes(), 'lxml')
            headers = s.find_all(["h1", "h2", "h3"])
            if not headers:
                continue

            contents = s.find(class_="contents")
            contents.attrs.pop('class')
            contents = contents.wrap(s.new_tag('div', attrs={'class': 'contents'}))
            toc = s.new_tag('toc')
            contents.insert(0, toc)

            for header in headers:
                toc_entry = s.new_tag(header.name)
                toc_entry.append(header.text.strip('\n'))

                toc.append(toc_entry)

            full_path.write_text(str(s))