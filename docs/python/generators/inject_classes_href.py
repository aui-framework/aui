#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import re

from mkdocs.structure.files import Files, File

from docs.python.generators import regexes


def _extract_page_title(page: File):
    try:
        if m := regexes.PAGE_TITLE.match(page.content_string):
            return m.group(1)
    except UnicodeDecodeError:
        pass
    return None

mapping = None

def inject_classes_href(html: str, files: Files):
    global mapping
    if mapping is None:
        mapping = {_extract_page_title(i): i for i in files}

    def process():
        items = re.findall('(<[^>]*>|[\w\'_\.]+|.)', html, flags=re.S)
        ignore = False
        for i in items:
            if not ignore: # skip contents of <a>
                if not i.startswith("<"): # skip all xml tags
                    if page := mapping.get(i):
                        yield f'<a href="/{page.url}">{i}</a>'
                        continue
            if i.startswith("<a "):
                ignore = True
            elif i == "</a>":
                ignore = False


            yield i

    html_out = "".join([i for i in process()])
    return html_out
