#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import re
from urllib.parse import urlsplit, urlunsplit

from mkdocs import utils
from mkdocs.structure.files import Files, File
from mkdocs.structure.pages import Page

from docs.python.generators import regexes, autorefs, index


def inject_classes_href(html: str, page: Page, files: Files):
    """
    Generates automatic references for words with no explicit linking. This is especially useful in code snippets where
    Markdown [linking syntax]() is not viable.

    Failure to link is not an error. So, if you want to benefit from link checking, consider using [explicit] links in
    Markdown.
    """

    def process():
        items = re.findall('(<[^>]*>|[\w\'_\.]+|.)', html, flags=re.S)
        ignore = False
        for i in items:
            if not ignore: # skip contents of <a>
                if not i[0] in ["\n", "<", " ", "\t"]: # skip all xml tags
                    if entry := index.find_page(i):
                        if entry.containing_file != page.file: # skip refences to itself
                            # _RelativePathTreeprocessor
                            scheme, netloc, path, query, anchor = urlsplit(entry.url)
                            path = utils.get_relative_url(entry.containing_file.url, page.file.url)
                            url = urlunsplit(('', '', path, query, anchor))
                            yield f'<a href="{url}">{i}</a>'
                            continue
            for tag in ["a", "h1", "h2", "h3", "h4", "h5", "h6"]:
                if i.startswith(f"<{tag} "):
                    ignore = True
                elif i == f"</{tag}>":
                    ignore = False


            yield i

    html_out = "".join([i for i in process()])
    return html_out
