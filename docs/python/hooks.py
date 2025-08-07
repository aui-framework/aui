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
from pathlib import Path

import mkdocs
from mkdocs.config.defaults import MkDocsConfig
from mkdocs.structure.files import File, Files
from mkdocs.structure.nav import Navigation
from mkdocs.structure.pages import Page
from re import Match

from docs.python import common, regexes, examples_page

log = logging.getLogger('mkdocs')

def on_page_markdown(
        markdown: str, *, page: Page, config: MkDocsConfig, files: Files
):
    # Replace callback
    def replace_comment(match: re.Match):
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
        if type == "include":
            return _include(args)

        # Otherwise, raise an error
        raise RuntimeError(f"Unknown shortcode: {type} in {page}")

    markdown = re.sub(
        r"<!-- aui:([\w\-]+)(.*?) -->",
        replace_comment, markdown, flags=re.I | re.M
    )

    def replace_url(match: re.Match):
        referred_page_id = match.group(1)
        referred_page = files.src_paths.get(referred_page_id)
        if not referred_page:
            line = markdown[:match.endpos].count('\n') + 1
            log.warning(f"Unknown page: {referred_page_id} in {page.file.abs_src_path}:{line}")
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

    markdown = "\n".join([i for i in handle_lines(iter(markdown.splitlines()))])

    return markdown

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

def _include(args: str):
    args = re.match(r"(\S+)(.*)", args)
    file_path = Path(args.group(1))
    return f"""
```{common.determine_extension(file_path)} linenums="1"{args.group(2)}
{Path(file_path).read_text()}
```
"""


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