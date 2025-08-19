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

from docs.python.generators import regexes, index

log = logging.getLogger('mkdocs')


def handle_autorefs(markdown: str, page : Page, files: Files):
    global _mapping
    index.populate_mapping(files)

    def replace_url(referred_page_id: str):
        referred_page_id = referred_page_id.rstrip('()') # refs to functions might have trailing braces AString::first().
        entry = index.find_page(referred_page_id)
        if not entry:
            log.warning(f"Doc file '{page.file.abs_src_path}' contains an unrecognized explicit link to '{referred_page_id}'.")
            return f'<span style="background:red">unrecognized link to "{referred_page_id}"</span>'
        title = entry.title
        if "::" in title:
            # referring to a code entity - make code background automatically.
            title = f"`{title}`"
        return f"[{title}]({entry.url})"

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
