#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import logging
import re
from urllib.parse import urlsplit, urlunsplit

from mkdocs import utils
from mkdocs.structure.files import Files, File
from mkdocs.structure.pages import Page

from docs.python.generators import regexes, autorefs, index
from docs.python.generators.index import MappingEntry

log = logging.getLogger('mkdocs')

def _emit_reference(page: Page, entry: MappingEntry, display: str):
    if entry.containing_file.url == page.file.url: # skip refences to itself
        return False

    # _RelativePathTreeprocessor
    scheme, netloc, path, query, anchor = urlsplit(entry.url)
    path = utils.get_relative_url(entry.containing_file.url, page.file.url)
    url = urlunsplit(('', '', path, query, anchor))
    return f'<a href="{url}">{display}</a>'

def inject_classes_href(html: str, page: Page, files: Files):
    """
    Generates automatic references for words with no explicit linking. This is especially useful in code snippets where
    Markdown [linking syntax]() is not viable.

    Failure to link is not an error. So, if you want to benefit from link checking, consider using [explicit] links in
    Markdown.
    """

    def process():
        items = regexes.HREF_INJECT.findall(html)
        ignore = False
        for i in items:
            i = i[0]
            if not ignore: # skip contents of <a>
                if not i[0] in ["\n", "<", " ", "\t"]: # skip all xml tags
                    shortened = i.replace("(", "").replace(")", "")
                    if entry := index.find_page(shortened):
                        if s := _emit_reference(page=page,entry=entry,display=i):
                            yield s
                            continue
                    elif "::" in i:
                        # try to emit a reference to a class instead.
                        shortened = i[:i.find(':')]
                        if entry := index.find_page(shortened):
                            if s := _emit_reference(page=page,entry=entry,display=shortened):
                                yield s
                                yield i[len(shortened):] # emit remaining part as we emitted the shortened version only.
                                continue

            for tag in ["a", "h1", "h2", "h3", "h4", "h5", "h6"]:
                if i.startswith(f"<{tag} "):
                    ignore = True
                elif i == f"</{tag}>":
                    ignore = False


            yield i

    html_out = "".join([i for i in process()])
    return html_out
