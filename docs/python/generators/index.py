#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import logging

from mkdocs.structure.files import File, Files

from docs.python.generators import regexes

log = logging.getLogger('mkdocs')

_mapping = None


class MappingEntry:
    def __init__(self, title: str, url: str, containing_file: File):
        self.title = title
        self.url = url
        self.containing_file = containing_file

def populate_mapping(files: Files):
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
                    _mapping[heading_id] = MappingEntry(title=heading_title, url=f"{file.src_uri}#{heading_id}", containing_file=file)

                if m := regexes.INDEX_ALIAS.match(line_contents):
                    heading_id = m.group(1)
                    _mapping[heading_id] = MappingEntry(title=heading_id, url=f"{file.src_uri}", containing_file=file)


        except UnicodeDecodeError:
            pass


def find_page(name: str) -> MappingEntry | None:
    global _mapping
    return _mapping.get(name)

