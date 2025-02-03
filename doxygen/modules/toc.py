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

def tag(name, content='text', **kwargs):
    global s
    tag = s.new_tag(name, **kwargs)
    tag.append(content)
    return tag

def run():
    for root, dirs, files in os.walk(CONFIG['output']):
        for file in files:
            if not file.endswith(".html"):
                continue

            full_path = Path(root) / file
            raw_contents = full_path.read_bytes().decode('utf-8')
            if "contents-rails-left" in raw_contents:
                continue
            global s
            s = soup(raw_contents, 'lxml')
            headers = s.find_all(["h1", "h2", "h3"])

            contents = s.find(class_="contents")
            if not contents:
                continue

            contents.attrs['class'] = 'contents-rails-left'
            contents = contents.wrap(s.new_tag('div'))
            contents.attrs['class'] = 'contents aui-toc-contents'
            rails_right = s.new_tag('div', attrs={'class': 'contents-rails-right'})
            contents.append(rails_right)

            toc = s.new_tag('div', attrs={'class': 'aui-toc'})

            toc.append(tag('p', content='Contents'))

            for header in headers:

                text = header.text.strip('\n')

                a = header.find_next("a")
                href = ""
                if a:
                    a.unwrap()
                    a.append("#")
                    a.attrs['class'] = "aui-toc-hash"
                    a.attrs['style'] = "color: var(--fragment-comment) !important"

                    if "autotoc" in a.attrs.get('id', 'autotoc'):
                        a.attrs['id'] = text.lower().replace(' ', '-') # generate betterid

                    href = a.attrs['href'] = f"#{a.attrs['id']}"

                    header.append(a) # to the end

                if "Documentation" in text:
                    continue
                if "memtitle" in header.attrs.get('class', ''): # drops function descriptions
                    continue

                tag_name = header.name
                is_doxygen_group_header = "groupheader" in header.attrs.get('class', '')
                if is_doxygen_group_header:
                    tag_name = "h1"

                toc.append(tag(tag_name, content=tag('a', content=text, href=href)))

            if not file == "index.html" and not len(headers) < 2:
                rails_right.append(toc)

            full_path.write_text(str(s))