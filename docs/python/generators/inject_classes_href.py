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
from mkdocs.structure.pages import Page

from docs.python.generators import regexes


def _extract_page_title(page: File):
    try:
        if m := regexes.PAGE_TITLE.match(page.content_string):
            return m.group(1)
    except UnicodeDecodeError:
        pass
    return None

mapping = None

def inject_classes_href(html: str, page: Page, files: Files):
    """
    Generates automatic references for words with no explicit linking. This is especially useful in code snippets where
    Markdown [linking syntax]() is not viable.

    Failure to link is not an error. So, if you want to benefit from link checking, consider using [explicit] links in
    Markdown.
    """
    global mapping
    if mapping is None:
        mapping = {_extract_page_title(i): i for i in files}

    def process():
        items = re.findall('(<[^>]*>|[\w\'_\.]+|.)', html, flags=re.S)
        ignore = False
        for i in items:
            if not ignore: # skip contents of <a>
                if not i.startswith("<"): # skip all xml tags
                    if file := mapping.get(i):
                        if file != page.file: # skip refences to itself
                            yield f'<a href="/{file.url}">{i}</a>'
                            continue
            for tag in ["a", "h1", "h2", "h3", "h4", "h5", "h6"]:
                if i.startswith(f"<{tag} "):
                    ignore = True
                elif i == f"</{tag}>":
                    ignore = False


            yield i

    html_out = "".join([i for i in process()])
    return html_out
