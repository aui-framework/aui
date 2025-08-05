#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import logging
import posixpath
import re

from mkdocs.config.defaults import MkDocsConfig
from mkdocs.structure.files import File, Files
from mkdocs.structure.pages import Page
from re import Match

import examples_page

log = logging.getLogger('mkdocs')


def on_page_markdown(
        markdown: str, *, page: Page, config: MkDocsConfig, files: Files
):
    # Replace callback
    def replace(match: re.Match):
        type, args = match.groups()
        args = args.strip()
        if type == "example-file-count":
            return _badge_for_file_count(args, page, files)
        if type == "example":
            return examples_page.example(args)
        if type == "examples":
            return examples_page.examples(args)
        if type == "icon":
            return _badge_for_icon(args)

        # Otherwise, raise an error
        raise RuntimeError(f"Unknown shortcode: {type} in {page}")

    # Find and replace all external asset URLs in current page
    return re.sub(
        r"<!-- aui:([\w\-]+)(.*?) -->",
        replace, markdown, flags=re.I | re.M
    )

def _badge(icon: str, text: str = "", type: str = "", tooltip: str = ""):
    classes = f"mdx-badge mdx-badge--{type}" if type else "mdx-badge"
    return "".join([
        f"<span class=\"{classes}\" title=\"{tooltip}\">",
        *([f"<span class=\"mdx-badge__icon\">{icon}</span>"] if icon else []),
        *([f"<span class=\"mdx-badge__text\">{text}</span>"] if text else []),
        f"</span>",
    ])

def _badge_for_icon(icon: str):
    return _badge(
        icon = f":{icon}:",
    )

def _badge_for_file_count(text: str, page: Page, files: Files):
    if text == "0":
        return _badge(
            icon = ":octicons-link-external-16:",
            tooltip = "External repository"
        )

    return _badge(
        icon = ":material-file-multiple-outline:",
        text = text,
        tooltip = "File count"
    )