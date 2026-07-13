#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import logging
import sys
from pathlib import Path

import mkdocs.plugins
from mkdocs.config.defaults import MkDocsConfig
from mkdocs.structure.files import Files
from mkdocs.structure.pages import Page

l = str(Path.cwd())
sys.path.append(l)

from docs.python.generators import index, autorefs
from docs.python.generators.comment_macros import handle_comment_macros
from docs.python.generators.inject_classes_href import inject_classes_href

log = logging.getLogger('mkdocs')

def on_files(files: Files, config: MkDocsConfig):
    files = Files([i for i in files if not "python" in str(i.src_path)])

    # hack: perform macro handling and indexing at early stage.
    for file in files.documentation_pages():
        file.content_string = handle_comment_macros(file.content_string, file)
        index.populate_mapping(file.content_string, file)

    return files

def on_page_markdown(
        markdown: str, *, page: Page, config: MkDocsConfig, files: Files
):
    markdown = autorefs.handle_autorefs(markdown, page, files)

    return markdown


def on_page_content(html: str, page: Page, config: MkDocsConfig, files: Files):
    html = inject_classes_href(html, page, files)

    return html


import json
import re

def on_post_build(config):
    """
    Post-build hook to filter the search index, removing AUI_DECLARATIVE_FOR
    from pages that aren't primarily about AForEachUI/AUI_DECLARATIVE_FOR.

    This prevents the mkdocs search bar from returning unrelated pages when
    users search for "AUI_DECLARATIVE_FOR" — pages that happen to contain it
    in code examples but whose topic is something else entirely.
    """
    index_path = Path(config['site_dir']) / 'search' / 'search_index.json'
    if not index_path.exists():
        log.warning("search_index.json not found at %s, skipping search filter", index_path)
        return

    try:
        with open(index_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        log.error("Failed to load search index: %s", e)
        return

    # Pages whose URL path contains these tokens are considered relevant for
    # AUI_DECLARATIVE_FOR search results.
    RELEVANT_LOCATION_TOKENS = ['aforeachui', 'aui_declarative_for']

    pattern = re.compile(r'\bAUI_DECLARATIVE_FOR\b')
    modified = 0
    for doc in data.get('docs', []):
        location = (doc.get('location') or '').lower()
        text = doc.get('text', '')

        is_relevant = any(tok in location for tok in RELEVANT_LOCATION_TOKENS)
        if not is_relevant and pattern.search(text):
            doc['text'] = pattern.sub('', text)
            modified += 1

    if modified:
        log.info("Filtered AUI_DECLARATIVE_FOR from %d unrelated pages in search index", modified)
        try:
            with open(index_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, ensure_ascii=False)
        except Exception as e:
            log.error("Failed to write filtered search index: %s", e)

