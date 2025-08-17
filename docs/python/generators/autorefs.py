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

    def replace_url(referred_page_id: str):
        referred_page = find_page(referred_page_id)
        if not referred_page:
            log.warning(f"Doc file '{page.file.abs_src_path}' contains an unrecognized explicit link to '{referred_page_id}'.")
            return f'<span style="background:red">unrecognized link to {referred_page_id}</span>'
        title = _extract_page_title(referred_page)
        return f"[{title}]({page.file.src_uri})"

    def tokenize():
        iterator = iter(markdown)
        for i in iterator:
            match i:
                case '`':
                    i = next(iterator)
                    if i == '`':
                        i = next(iterator)
                        if i == '`':
                            yield '```'
                        else:
                            yield '`'
                            yield '`'
                            yield i
                    else:
                        yield '`'
                        yield i
                case _:
                    yield i

    def parse():
        iterator = iter(tokenize())
        state_code = False
        for i in iterator:
            if i == '```' or i == '`':
                yield i
                state_code = not state_code
            elif not state_code and i == '[':
                link_first_part = ''
                for i in iterator:
                    if i == ']':
                        break
                    link_first_part += i
                try:
                    i = next(iterator)
                except StopIteration:
                    i = ''
                if i == '(':
                    # just a regular link, no special treatment
                    yield f"[{link_first_part}]("
                else:
                    # an explicit link in format [AObject], where AObject is a destination classname/page name.
                    # this is not a standard Markdown though.
                    yield replace_url(link_first_part)
                    yield i

            else:
                yield i



    return "".join([i for i in parse()])
