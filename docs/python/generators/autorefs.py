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

class MappingEntry:
    def __init__(self, title: str, url: str, containing_file: File):
        self.title = title
        self.url = url
        self.containing_file = containing_file

def _populate_mapping(files: Files):
    global _mapping
    if _mapping is not None:
        return
    _mapping = {}

    for file in files:
        try:
            page_title = None
            if m := regexes.PAGE_TITLE.match(file.content_string):
                page_title = m.group(1)
                _mapping[page_title] = MappingEntry(title=page_title, url=file.src_uri, containing_file=file)
                _mapping[file.src_uri] = MappingEntry(title=page_title, url=file.src_uri, containing_file=file)

            state_code = False
            for line_number, line_contents in enumerate(file.content_string.splitlines()):
                if "```" in line_contents:
                    state_code = not state_code
                    continue

                if state_code:
                    continue
                if line_contents.startswith("# ") and line_number != 0:
                    # several H1 headings breaks TOC.
                    log.warning(f"Doc file '{file.abs_src_path}':{line_number+1} contains several H1 headings")

                if m := regexes.HEADING_ANCHOR.match(line_contents):
                    heading_title = m.group(1)
                    heading_id = m.group(3)
                    _mapping[heading_id] = MappingEntry(title=heading_title, url=f"{file.url}#{heading_id}", containing_file=file)

                if m := regexes.INDEX_ALIAS.match(line_contents):
                    heading_id = m.group(1)
                    _mapping[heading_id] = MappingEntry(title=heading_id, url=f"{file.url}", containing_file=file)


        except UnicodeDecodeError:
            pass


def find_page(name: str) -> MappingEntry | None:
    global _mapping
    return _mapping.get(name)


def handle_autorefs(markdown: str, page : Page, files: Files):
    global _mapping
    _populate_mapping(files)

    def replace_url(referred_page_id: str):
        referred_page = find_page(referred_page_id)
        if not referred_page:
            log.warning(f"Doc file '{page.file.abs_src_path}' contains an unrecognized explicit link to '{referred_page_id}'.")
            return f'<span style="background:red">unrecognized link to {referred_page_id}</span>'
        return f"[{referred_page.title}]({referred_page.url})"

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
