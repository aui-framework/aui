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
from pathlib import Path

from mkdocs.structure.files import Files
from mkdocs.structure.pages import Page

from docs.python.generators import parse_tests, examples_page, common

log = logging.getLogger('mkdocs')

def handle_comment_macros(markdown: str, page: Page, files: Files):
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
        if type == "snippet":
            return _snippet(args)
        if type == "parse_tests":
            return parse_tests.parse_tests(Path(args))
        if type == "experimental":
            return _experimental(args)
        if type == "index_alias":
            return match.group(0) # handled at autorefs stage
        # Otherwise, raise an error
        raise RuntimeError(f"Unknown shortcode: {type} in {page}")

    return re.sub(
        r"<!-- aui:([\w\-]+)(.*?) -->",
        replace_comment, markdown, flags=re.I | re.M
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

def _include(args: str):
    args = re.match(r"(\S+)(.*)", args)
    file_path = Path(args.group(1))
    return f"""
```{common.determine_extension(file_path)} linenums="1"{args.group(2)}
{Path(file_path).read_text()}
```
"""


def _snippet(args: str):
    args = re.match(r"(\S+) (\S+)(.*)", args)
    file_path = Path(args.group(1))
    section_name = Path(args.group(2))

    def find_section():
        iterator = iter(Path(file_path).read_text().split('\n'))
        for i in iterator:
            if f"[{section_name}]" in i:
                break
        for i in iterator:
            if f"[{section_name}]" in i:
                break
            yield i
    section = [i for i in find_section()]
    if not section:
        log.warning(f"Can't find section '{section_name}' in {file_path}")
        section = f"/* can't find section: {args} */"

    section = "\n".join(common.strip_indentation(section))


    return f"""
```{common.determine_extension(file_path)} linenums="1"{args.group(3)}
{section}
```
"""


def _experimental(name):
    return """
!!! bug "Experimental Feature"

    This API is experimental. Experimental APIs are likely to contain bugs, might be changed or removed in the future.
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
