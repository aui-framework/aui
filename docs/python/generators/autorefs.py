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

from mkdocs.structure.files import Files, File
from mkdocs.structure.pages import Page

from docs.python.generators import regexes

log = logging.getLogger('mkdocs')


_mapping = None


def _extract_page_title(page: File):
    try:
        if m := regexes.PAGE_TITLE.match(page.content_string):
            return m.group(1)
    except UnicodeDecodeError:
        pass
    return None
def find_page(name: str) -> File | None:
    global _mapping
    return _mapping.get(name)


def handle_autorefs(markdown: str, page : Page, files: Files):
    global _mapping
    if _mapping is None:
        _mapping = {_extract_page_title(i): i for i in files} | { i.src_uri : i for i in files }

    def replace_url(match: re.Match):
        referred_page_id = match.group(1)
        referred_page = find_page(referred_page_id)
        if not referred_page:
            line = markdown[:match.endpos].count('\n') + 1
            log.warning(f"Doc file '{page.file.abs_src_path}' (line {line}) contains an unrecognized explicit link to '{referred_page_id}'.")
            return f'<span style="background:red">unrecognized link to {referred_page_id}</span>'
        title = _extract_page_title(referred_page)
        return f"[{title}](/{page.url})"

    def handle_lines(iterator: iter):
        for line in iterator:
            if "```" in line:
                yield line
                for line in iterator:
                    yield line
                    if "```" in line:
                        break
            else:
                line = re.sub(
                    r"\[(.+)\](?![\(])",
                    replace_url, line, flags=re.I | re.M
                )
                yield line

    return "\n".join([i for i in handle_lines(iter(markdown.splitlines()))])
