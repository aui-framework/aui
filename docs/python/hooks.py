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
import sys
from pathlib import Path

from docs.python.generators.autorefs import handle_autorefs
from docs.python.generators.comment_macros import handle_comment_macros
from docs.python.generators.inject_classes_href import inject_classes_href

sys.path.append(str(Path.cwd()))

from mkdocs.config.defaults import MkDocsConfig
from mkdocs.structure.files import Files
from mkdocs.structure.pages import Page


log = logging.getLogger('mkdocs')

def on_page_markdown(
        markdown: str, *, page: Page, config: MkDocsConfig, files: Files
):
    markdown = handle_comment_macros(markdown, page, files)
    markdown = handle_autorefs(markdown, page, files)

    return markdown


def on_page_content(html: str, page: Page, config: MkDocsConfig, files: Files):
    html = inject_classes_href(html, page, files)

    return html


def on_files(files: Files, config: MkDocsConfig):
    return Files([i for i in files if not "python" in str(i.src_path)])
