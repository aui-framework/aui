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

from mkdocs.structure.files import Files
from mkdocs.structure.pages import Page

from docs.python.generators import regexes

log = logging.getLogger("autorefs")

def handle_autorefs(markdown: str, page : Page, files: Files):
    def replace_url(match: re.Match):
        referred_page_id = match.group(1)
        referred_page = files.src_paths.get(referred_page_id)
        if not referred_page:
            line = markdown[:match.endpos].count('\n') + 1
            log.warning(f"Doc file '{page.file.abs_src_path}' (line {line}) contains an unrecognized link to '{referred_page_id}'.")
            return f"==broken link to {referred_page_id}=="
        title = regexes.PAGE_TITLE.match(referred_page.content_string).group(1)
        return f"[{title}]({referred_page_id})"

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
                    r"\[(.+\.md)\](?![\(])",
                    replace_url, line, flags=re.I | re.M
                )
                yield line

    return "\n".join([i for i in handle_lines(iter(markdown.splitlines()))])
