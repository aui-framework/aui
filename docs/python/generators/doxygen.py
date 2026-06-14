#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import logging
import re

import mkdocs_gen_files

from docs.python.generators import cpp_parser, common, group_page, examples_page, regexes
from docs.python.generators.cpp_parser import DoxygenEntry, CppClass
from pathlib import Path
from docs.python.generators.examples_helpers import (
    examples_for_symbol_with_snippets as _examples_for_symbol_with_snippets,
    examples_for_symbol as _examples_for_symbol,
    compute_hl_lines as _compute_hl_lines,
    filter_examples_by_relevance as _filter_examples_by_relevance,
    dedupe_examples_list as _dedupe_examples_list,
)
from docs.python.generators.examples_helpers import _find_unquoted_word_on_nontrivial_line

log = logging.getLogger('mkdocs')


def _has_unquoted_match(snippet: str, names: list[str]) -> bool:
    """Return True if any of the names appears in snippet outside double quotes."""
    if not snippet:
        return False
    import re
    for name in names:
        if not name:
            continue
        for m in re.finditer(re.escape(name), snippet):
            i = m.start()
            # find nearest quote before and after the match
            before = snippet.rfind('"', 0, i)
            after = snippet.find('"', i)
            # if there's no surrounding pair of quotes containing the match,
            # consider it unquoted
            if before == -1 or after == -1 or not (before < i < after):
                return True
    return False



def _extract_example_names(parse_entry):
    t = [parse_entry.namespaced_name(), parse_entry.name]
    def _extract_aliases(doc: str):
        def _impl():
            for i in doc.splitlines():
                if m := regexes.INDEX_ALIAS.match(i):
                    yield m.group(1)
                    continue
                if m := regexes.STEAL_DOCUMENTATION.match(i):
                    yield m.group(1)
                    continue
        for i in _impl():
            yield i
            for omit in common.OMIT_NAMESPACES:
                namespaced = f"{omit}::"
                if i.startswith(namespaced):
                    yield i[len(namespaced):]
    t += _extract_aliases(parse_entry.doc)
    return t


def _format_token_sequence(tokens: list[str]):
    output = " ".join(tokens)
    for i in [",", "&&", ">", "&", "*"]:
        output = output.replace(f" {i} ", f"{i} ")
    for i in "()<[]:":
        output = output.replace(f"{i} ", i)
        output = output.replace(f" {i}", i)
    return output


def embed_doc(nested, fos, names_to_search_examples=[], printed_example_pairs=set()):
    doxygen = common.parse_doxygen(nested.doc)

    # todo add a check for @brief to exist
    for i in [i for i in doxygen if i[0] == '@brief']:
        print(i[1], file=fos)

    for i in doxygen:
        if i[0] in ['', '@details']:
            print(f'{i[1]}', file=fos)

            # Examples for nested types
            exs = _examples_for_symbol_with_snippets(names_to_search_examples, anchors=names_to_search_examples, examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None))
            exs = _dedupe_examples_list(exs)
            exs = _filter_examples_by_relevance(exs, names_to_search_examples, strict=True)
            if not exs:
                fallback_list = _dedupe_examples_list(_examples_for_symbol(names_to_search_examples, examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None)))
                exs = _filter_examples_by_relevance(fallback_list, names_to_search_examples, strict=False)
            if exs:
                print('\n**Examples:**\n', file=fos)
                for ex in exs:
                    if not ex or 'src' not in ex or not ex.get('snippet'):
                        continue
                    try:
                        src_rel = ex['src'].relative_to(Path.cwd())
                    except Exception:
                        src_rel = ex['src']
                    pair = (ex.get('id'), str(src_rel))
                    if pair in printed_example_pairs:
                        continue
                    printed_example_pairs.add(pair)
                    extension = common.determine_extension(ex['src'])
                    # compute hl_lines using nested type tokens
                    tokens = [i for i in names_to_search_examples]
                    snippet = ex.get('snippet','') or ''
                    try:
                        if 'AUI_DECLARATIVE_FOR' in snippet and 'AUI_DECLARATIVE_FOR' not in tokens:
                            tokens.append('AUI_DECLARATIVE_FOR')
                    except Exception:
                        pass
                    hl = _compute_hl_lines(snippet, tokens)
                    hl_attr = f' hl_lines="{hl}"' if hl else ''
                    print(f"\n??? note \"{src_rel}\"", file=fos)
                    print(file=fos)
                    print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                    print(file=fos)
                    print(f"    ```{extension}{hl_attr}", file=fos)
                    for line in ex['snippet'].splitlines():
                        print(f"    {line}", file=fos)
                    print(f"    ```", file=fos)

    # enums - see APath::DefaultPath
    if hasattr(nested, 'enum_values'):
        print('<table markdown>', file=fos)
        print('<tr markdown>', file=fos)
        print('<th>Constant</th>', file=fos)
        print('<th>Description</th>', file=fos)
        print('</tr>', file=fos)
        for v in nested.enum_values:
            print('<tr markdown>', file=fos)
            print('<td markdown>', file=fos)
            print(f'`#!cpp {nested.name}::{v[0]}`', file=fos)
            print('</td>', file=fos)
            print('<td markdown>', file=fos)
            for i in common.parse_doxygen(v[1]):
                print(i[1], file=fos)
            print('</td>', file=fos)
            print('</tr>', file=fos)

        print('</table>', file=fos)

    # nested struct - see AUpdater::InstallCmdline, AButton
    if hasattr(nested, 'fields'):
        if nested.fields:
            print('<dl style="padding-left:1em" markdown>', file=fos)
            for v in nested.fields:
                print(f'<dt markdown>`#!cpp {v.type_str} {v.name}`</dt>', file=fos)
                print(f'<dd markdown>', file=fos)
                for i in common.parse_doxygen(v.doc):
                    print(i[1], file=fos)
                print('</dd>', file=fos)
            print('</dl>', file=fos)
        else:
            print('\n\n_Empty structure._', file=fos)
    pass


def gen_pages():
    for parse_entry in cpp_parser.index:
        if hasattr(parse_entry, 'doc'):
            if "<!-- aui:no_dedicated_page -->" in parse_entry.doc:
                continue

        if isinstance(parse_entry, DoxygenEntry):
            group_page.handle_doxygen_entry(parse_entry)
            continue

        slugged_name = parse_entry.namespaced_name().lower().replace('::', '_')
        parse_entry.page_url = f'{slugged_name}.md'
        with mkdocs_gen_files.open(parse_entry.page_url, 'w') as fos:
            def _parse_page_metadata():
                """
                Finds the following block in the doxygen comment:

                ---
                title: Custom title
                icon: custom-icon
                ---

                https://squidfunk.github.io/mkdocs-material/reference/#setting-the-page-icon
                """

                iterator = iter(parse_entry.doc.splitlines())
                for i in iterator:
                    if i == '---':
                        print('---', file=fos)
                        for j in iterator:
                            print(j, file=fos)

                            if m := re.match('title: (.+)', j):
                                # hack: use the overrided name if it exists
                                parse_entry.overview_page_title = m.group(1)

                            if j == '---':
                                print('', file=fos)
                                return
            _parse_page_metadata()

            print(f'# {parse_entry.namespaced_name()}', file=fos)
            print(f'', file=fos)
            doxygen = common.parse_doxygen(parse_entry.doc)

            page_examples: list[dict] = []
            # per-page set to avoid printing same example (id, src) multiple times
            printed_example_pairs: set = set()

            # Use centralized examples lookup from examples_helpers; pass the
            # examples lists/index from `examples_page` so the helper can use
            # the existing index when available.


            include_dir = parse_entry.location
            while include_dir.name != 'src':
                if include_dir == include_dir.parent:
                    break
                include_dir = include_dir.parent

            module_name = str(include_dir.parent.name).replace('.', '::')

            for type_entry in [i for i in doxygen if i[0] == '@brief']:
                print(type_entry[1], file=fos)

            # For non-class symbols (macros, free functions, enums), collect examples to print after Detailed Description
            if not isinstance(parse_entry, CppClass):
                try:
                    names_to_search = []
                    if hasattr(parse_entry, 'namespaced_name'):
                        names_to_search.append(parse_entry.namespaced_name())
                    if hasattr(parse_entry, 'name'):
                        names_to_search.append(parse_entry.name)
                    exs = _examples_for_symbol_with_snippets(names_to_search, anchors=None, examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None))
                    # If filtering removed all candidates but we have precomputed
                    # class_examples (discovered by a permissive index search),
                    # fall back to them so examples found on other pages are
                    # nevertheless printed on the class page.
                    if not exs and class_examples:
                        try:
                            exs = _dedupe_examples_list(class_examples)
                        except Exception:
                            pass

                    if exs:
                        # rank examples by strong signals: macro usage, header include, unquoted name
                        def _example_priority(e):
                            try:
                                text = Path(e.get('src')).read_text(encoding='utf-8', errors='ignore')
                            except Exception:
                                text = ''
                            snippet = e.get('snippet','') or ''
                            # strongest: macro invocation AUI_DECLARATIVE_FOR
                            if re.search(r"\bAUI_DECLARATIVE_FOR\s*\(", snippet) or re.search(r"\bAUI_DECLARATIVE_FOR\s*\(", text):
                                return 0
                            # header include for AForEachUI
                            if re.search(r"#include\s+[<\"]AUI/View/AForEachUI.h[>\"]", text):
                                return 1
                            # unquoted AForEachUI occurrence on nontrivial line
                            try:
                                from docs.python.generators.examples_helpers import _find_unquoted_word_on_nontrivial_line
                                if any(_find_unquoted_word_on_nontrivial_line(n, text) for n in names_to_search if n):
                                    return 2
                            except Exception:
                                pass
                            # prefer snippets that contain macro anchors
                            try:
                                if any(re.search(re.escape(a), snippet) for a in (names_to_search or [])):
                                    return 3
                            except Exception:
                                pass
                            return 10

                        # sort examples by computed priority
                        exs.sort(key=_example_priority)
                        # log per-example priority for debugging
                        try:
                            for e in exs:
                                try:
                                    p = _example_priority(e)
                                except Exception:
                                    p = None
                                log.info(f"example candidate: id={e.get('id')} title={e.get('title')} priority={p}")
                        except Exception:
                            pass
                        # if any has strong signal (priority < 10), keep only those with strong signal
                        if any(_example_priority(e) < 10 for e in exs):
                            exs = [e for e in exs if _example_priority(e) < 10]
                        # if any example contains an unquoted match in its source file
                        # (and the match is on a non-trivial line), drop examples
                        # whose matches are exclusively inside quoted strings.
                        try:
                            # expand simple macro alias pair used by helpers so we check both forms
                            check_names = [n for n in (names_to_search or []) if n]
                            try:
                                if 'AForEachUI' in check_names and 'AUI_DECLARATIVE_FOR' not in check_names:
                                    check_names.append('AUI_DECLARATIVE_FOR')
                                if 'AUI_DECLARATIVE_FOR' in check_names and 'AForEachUI' not in check_names:
                                    check_names.append('AForEachUI')
                            except Exception:
                                pass

                            any_unquoted = False
                            for e in exs:
                                try:
                                    text = Path(e.get('src')).read_text(encoding='utf-8', errors='ignore')
                                except Exception:
                                    text = ''
                                if any(_find_unquoted_word_on_nontrivial_line(n, text) for n in check_names if n):
                                    any_unquoted = True
                                    break
                            if any_unquoted:
                                # keep examples that contain at least one unquoted occurrence of any checked name
                                filtered = []
                                for e in exs:
                                    try:
                                        text = Path(e.get('src')).read_text(encoding='utf-8', errors='ignore')
                                    except Exception:
                                        text = ''
                                    if any(_find_unquoted_word_on_nontrivial_line(n, text) for n in check_names if n):
                                        filtered.append(e)
                                exs = filtered
                        except Exception:
                            pass
                        page_examples.extend(exs)
                except Exception:
                    pass

            has_detailed_description = bool([i for i in doxygen if i[0] == '@details'])

            print('<table>', file=fos)
            for type_entry in [('Header:', f'<code>#include &lt;{parse_entry.location.relative_to(include_dir)}&gt;</code>'), ('CMake:', f'<code>aui_link(my_target PUBLIC {module_name})</code>')]:
                print(f'<tr><td>{type_entry[0]}</td><td>{type_entry[1]}</td></tr>', file=fos)
            print('</table>', file=fos)

            if hasattr(parse_entry, 'definition'):
                print('## Definition', file=fos)
                print('```cpp', file=fos)
                print(parse_entry.definition, file=fos)
                print('```\n', file=fos)

            if has_detailed_description:
                print('## Detailed Description', file=fos)
                for type_entry in [i for i in doxygen if i[0] == '@details']:
                    print(type_entry[1], file=fos)

            # Print page-level examples immediately after Detailed Description
            if page_examples:
                printed_example_ids = set()
                print('\n## Examples', file=fos)
                for ex in page_examples:
                    exid = ex.get('id')
                    if exid in printed_example_ids:
                        continue
                    printed_example_ids.add(exid)
                    try:
                        src_rel = ex['src'].relative_to(Path.cwd())
                    except Exception:
                        src_rel = ex['src']
                    pair = (ex.get('id'), str(src_rel))
                    if pair in printed_example_pairs:
                        continue
                    printed_example_pairs.add(pair)
                    extension = common.determine_extension(ex['src'])
                    # compute hl_lines for this snippet using name tokens
                    tokens = []
                    if hasattr(parse_entry, 'namespaced_name'):
                        tokens.append(parse_entry.namespaced_name())
                    if hasattr(parse_entry, 'name'):
                        tokens.append(parse_entry.name)
                    snippet = ex.get('snippet','') or ''
                    try:
                        # highlight macro invocation line when present
                        if 'AUI_DECLARATIVE_FOR' in snippet and 'AUI_DECLARATIVE_FOR' not in tokens:
                            tokens.append('AUI_DECLARATIVE_FOR')
                        if any(t == 'AForEachUI' for t in tokens) and 'AUI_DECLARATIVE_FOR' not in tokens:
                            tokens.append('AUI_DECLARATIVE_FOR')
                    except Exception:
                        pass
                    hl = _compute_hl_lines(snippet, tokens)
                    hl_attr = f' hl_lines="{hl}"' if hl else ''
                    print(f"??? note \"{src_rel}\"", file=fos)
                    print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                    print(f"    ```{extension}{hl_attr}", file=fos)
                    for line in ex['snippet'].splitlines():
                        print(f"    {line}", file=fos)
                    print(f"    ```", file=fos)

            # collect class-level examples (don't print here; used as fallback per-method)
            class_examples = []
            try:
                top_names = []
                if hasattr(parse_entry, 'namespaced_name'):
                    top_names.append(parse_entry.namespaced_name())
                if hasattr(parse_entry, 'name'):
                    top_names.append(parse_entry.name)
                class_examples = _examples_for_symbol(top_names, examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None)) or []
            except Exception:
                class_examples = []
                pass

            # For classes: print class-level examples once (used as fallback reference).
            if isinstance(parse_entry, CppClass) and class_examples:
                try:
                    filter_names = _extract_example_names(parse_entry)
                    exs = _examples_for_symbol_with_snippets(filter_names, anchors=filter_names+[parse_entry.name.split('::')[-1] if '::' in parse_entry.namespaced_name() else None], examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None))
                    exs = _dedupe_examples_list(exs)
                    # include known macro alias in the filter names so macro-only
                    # snippets (AUI_DECLARATIVE_FOR) are considered relevant for
                    # AForEachUI class pages
                    try:
                        if 'AForEachUI' in filter_names and 'AUI_DECLARATIVE_FOR' not in filter_names:
                            filter_names.append('AUI_DECLARATIVE_FOR')
                        if 'AUI_DECLARATIVE_FOR' in filter_names and 'AForEachUI' not in filter_names:
                            filter_names.append('AForEachUI')
                    except Exception:
                        pass
                    exs = _filter_examples_by_relevance(exs, filter_names, strict=True)
                    # fallback to a relaxed filter if strict yields nothing (avoid empty class pages)
                    if not exs:
                        fallback_list = _dedupe_examples_list(_examples_for_symbol(_extract_example_names(parse_entry), examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None)))
                        exs = _filter_examples_by_relevance(fallback_list, filter_names, strict=False)
                    # Prefer examples with unquoted occurrences when available (based on source file, nontrivial lines)
                    try:
                        if exs:
                            # log initial class-level candidates
                            try:
                                log.info(f"class-level candidates before filtering: {[ (e.get('id'), e.get('title')) for e in exs ]}")
                            except Exception:
                                pass
                            # expand alias pair similar to helpers so macro-form usages are counted
                            check_names = [n for n in _extract_example_names(parse_entry) if n]
                            try:
                                if 'AForEachUI' in check_names and 'AUI_DECLARATIVE_FOR' not in check_names:
                                    check_names.append('AUI_DECLARATIVE_FOR')
                                if 'AUI_DECLARATIVE_FOR' in check_names and 'AForEachUI' not in check_names:
                                    check_names.append('AForEachUI')
                            except Exception:
                                pass

                            any_unquoted = False
                            per_example_unquoted = {}
                            for e in exs:
                                try:
                                    text = Path(e.get('src')).read_text(encoding='utf-8', errors='ignore')
                                except Exception:
                                    text = ''
                                has_unq = any(_find_unquoted_word_on_nontrivial_line(n, text) for n in check_names if n)
                                per_example_unquoted[e.get('id')] = has_unq
                                if has_unq:
                                    any_unquoted = True
                            try:
                                log.info(f"class-level per-example unquoted map: {per_example_unquoted}")
                            except Exception:
                                pass
                            if any_unquoted:
                                # keep examples that contain at least one unquoted occurrence of any checked name
                                filtered = []
                                for e in exs:
                                    try:
                                        text = Path(e.get('src')).read_text(encoding='utf-8', errors='ignore')
                                    except Exception:
                                        text = ''
                                    if any(_find_unquoted_word_on_nontrivial_line(n, text) for n in check_names if n):
                                        filtered.append(e)
                                exs = filtered
                                try:
                                    log.info(f"class-level candidates after unquoted-filter: {[ (e.get('id'), e.get('title')) for e in exs ]}")
                                except Exception:
                                    pass
                    except Exception:
                        pass
                    if exs:
                        print('\n## Examples', file=fos)
                        for ex in exs:
                            if not ex or 'src' not in ex or not ex.get('id'):
                                # skip malformed example entries
                                continue
                            try:
                                src_rel = ex['src'].relative_to(Path.cwd())
                            except Exception:
                                src_rel = ex['src']
                            pair = (ex.get('id'), str(src_rel))
                            if pair in printed_example_pairs:
                                continue
                            printed_example_pairs.add(pair)
                            extension = common.determine_extension(ex['src'])
                            # emit admonition header with no blank line after it
                            # compute hl_lines using class tokens
                            tokens = _extract_example_names(parse_entry)
                            snippet = ex.get('snippet', '') or ''
                            try:
                                if 'AUI_DECLARATIVE_FOR' in snippet and 'AUI_DECLARATIVE_FOR' not in tokens:
                                    tokens.append('AUI_DECLARATIVE_FOR')
                                if any(t == 'AForEachUI' for t in tokens) and 'AUI_DECLARATIVE_FOR' not in tokens:
                                    tokens.append('AUI_DECLARATIVE_FOR')
                            except Exception:
                                pass
                            hl = _compute_hl_lines(snippet, tokens)
                            hl_attr = f' hl_lines="{hl}"' if hl else ''
                            print(f"\n??? note \"{src_rel}\"", file=fos)
                            print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                            # print code block only when snippet is available
                            if snippet:
                                print(f"    ```{extension}{hl_attr}", file=fos)
                                for line in snippet.splitlines():
                                    print(f"    {line}", file=fos)
                                print(f"    ```", file=fos)
                except Exception:
                    pass

            # For non-class symbols: attempt to reuse class-level candidates
            # (examples that reference the symbol but may appear on other pages)
            # and print them here. Do this even if page-level examples exist so
            # canonical macro-form examples found elsewhere are surfaced on the
            # symbol's page (e.g., AForEachUI / AUI_DECLARATIVE_FOR).
            if not isinstance(parse_entry, CppClass) and class_examples:
                try:
                    # build names and anchors similar to class handling
                    anchors = []
                    top_names = []
                    if hasattr(parse_entry, 'namespaced_name'):
                        top_names.append(parse_entry.namespaced_name())
                        anchors.append(parse_entry.namespaced_name())
                    if hasattr(parse_entry, 'name'):
                        top_names.append(parse_entry.name)
                        anchors.append(parse_entry.name)

                    # Expand well-known macro alias so macro-form usages are considered
                    try:
                        if 'AForEachUI' in top_names and 'AUI_DECLARATIVE_FOR' not in top_names:
                            top_names.append('AUI_DECLARATIVE_FOR')
                            anchors.append('AUI_DECLARATIVE_FOR')
                        if 'AUI_DECLARATIVE_FOR' in top_names and 'AForEachUI' not in top_names:
                            top_names.append('AForEachUI')
                            anchors.append('AForEachUI')
                    except Exception:
                        pass

                    exs = _examples_for_symbol_with_snippets(top_names, anchors=anchors, examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None))
                    exs = _dedupe_examples_list(exs)
                    # be permissive for non-class symbol pages but still filter by relevance
                    exs = _filter_examples_by_relevance(exs, top_names, strict=False)
                    # if snippet-aware search returned nothing, fall back to the
                    # precomputed class_examples which may contain candidates
                    # discovered by a more permissive index search.
                    if not exs:
                        try:
                            exs = _dedupe_examples_list(class_examples)
                            exs = _filter_examples_by_relevance(exs, top_names, strict=False)
                        except Exception:
                            pass

                    # Prefer examples with unquoted occurrences when available
                    try:
                        if exs:
                            check_names = [n for n in top_names if n]
                            try:
                                if 'AForEachUI' in check_names and 'AUI_DECLARATIVE_FOR' not in check_names:
                                    check_names.append('AUI_DECLARATIVE_FOR')
                                if 'AUI_DECLARATIVE_FOR' in check_names and 'AForEachUI' not in check_names:
                                    check_names.append('AForEachUI')
                            except Exception:
                                pass

                            any_unquoted = False
                            for e in exs:
                                try:
                                    text = Path(e.get('src')).read_text(encoding='utf-8', errors='ignore')
                                except Exception:
                                    text = ''
                                if any(_find_unquoted_word_on_nontrivial_line(n, text) for n in check_names if n):
                                    any_unquoted = True
                                    break
                            if any_unquoted:
                                filtered = []
                                for e in exs:
                                    try:
                                        text = Path(e.get('src')).read_text(encoding='utf-8', errors='ignore')
                                    except Exception:
                                        text = ''
                                    if any(_find_unquoted_word_on_nontrivial_line(n, text) for n in check_names if n):
                                        filtered.append(e)
                                exs = filtered
                    except Exception:
                        pass

                    # If filtering produced nothing, fall back to any precomputed
                    # class_examples discovered earlier (less strict). This ensures
                    # canonical examples referenced elsewhere still surface on
                    # the class page.
                    if not exs:
                        try:
                            exs = _dedupe_examples_list(class_examples)
                            exs = _filter_examples_by_relevance(exs, [parse_entry.namespaced_name(), parse_entry.name], strict=False)
                        except Exception:
                            pass

                    if exs:
                        print('\n## Examples', file=fos)
                        for ex in exs:
                            if not ex or 'src' not in ex or not ex.get('id'):
                                continue
                            try:
                                src_rel = ex['src'].relative_to(Path.cwd())
                            except Exception:
                                src_rel = ex['src']
                            pair = (ex.get('id'), str(src_rel))
                            if pair in printed_example_pairs:
                                continue
                            printed_example_pairs.add(pair)
                            extension = common.determine_extension(ex['src'])
                            tokens = []
                            if hasattr(parse_entry, 'namespaced_name'):
                                tokens.append(parse_entry.namespaced_name())
                            if hasattr(parse_entry, 'name'):
                                tokens.append(parse_entry.name)
                            snippet = ex.get('snippet', '') or ''
                            hl = _compute_hl_lines(snippet, tokens)
                            hl_attr = f' hl_lines="{hl}"' if hl else ''
                            print(f"\n??? note \"{src_rel}\"", file=fos)
                            print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                            if snippet:
                                print(f"    ```{extension}{hl_attr}", file=fos)
                                for line in snippet.splitlines():
                                    print(f"    {line}", file=fos)
                                print(f"    ```", file=fos)
                except Exception:
                    pass

            def _render_invisible_header(toc, id, on_other_pages=None):
                # hack: present the header as invisible block. The header will still appear in TOC and
                # can be anchor referenced. Also, the markdown parser in index.py can pick up the heading.
                if not on_other_pages:
                    on_other_pages = toc
                print(f'<div style="position:absolute;opacity:0" markdown>', file=fos)
                print(f'### {toc} {"{"} #{id} {on_other_pages} {"}"} ', file=fos)
                print(f'</div>', file=fos)

            if hasattr(parse_entry, 'types'):
                types = [i for i in parse_entry.types]
                if types:
                    print('', file=fos)
                    print('## Public Types', file=fos)
                    for nested in types:
                        print('', file=fos)
                        print('---', file=fos)
                        print('', file=fos)
                        full_name = f"{parse_entry.namespaced_name()}::{nested.name}"
                        _render_invisible_header(toc=nested.name, id=full_name, on_other_pages=full_name)
                        print(f'`{nested.generic_kind} {parse_entry.namespaced_name()}::{nested.name}`', file=fos)
                        print(f'', file=fos)
                        names_to_search_examples = [full_name]
                        embed_doc(nested, fos, names_to_search_examples=names_to_search_examples, printed_example_pairs=printed_example_pairs)


            if hasattr(parse_entry, 'fields'):
                fields = [f for f in parse_entry.fields if f.visibility != 'private' and f.doc is not None]
                if fields:
                    print('## Public fields and Signals', file=fos)
                    for field in sorted(fields, key=lambda x: x.name):
                        print('---', file=fos)
                        full_name = f"{parse_entry.namespaced_name()}::{field.name}"
                        _render_invisible_header(toc=field.name, id=full_name, on_other_pages=full_name)
                        print(f'`#!cpp {field.type_str} {field.name}`\n\n', file=fos)
                        doxygen = common.parse_doxygen(field.doc)
                        for d in doxygen:
                            if d[0] != '@brief':
                                continue
                            print(d[1], file=fos)
                        print('', file=fos)
                        for d in doxygen:
                            if d[0] == '@brief':
                                continue
                            print(d[1], file=fos)
                        print(f'\n', file=fos)

                        # Examples for fields
                        names_to_search = [full_name, field.name]
                        exs = _examples_for_symbol_with_snippets(names_to_search, anchors=[full_name, field.name], examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None))
                        exs = _dedupe_examples_list(exs)
                        exs = _filter_examples_by_relevance(exs, names_to_search, strict=True)
                        if not exs:
                            fallback_list = _dedupe_examples_list(_examples_for_symbol(names_to_search, examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None)))
                            exs = _filter_examples_by_relevance(fallback_list, names_to_search, strict=False)
                        if exs:
                            print('## Examples', file=fos)
                            for ex in exs:
                                if not ex or 'src' not in ex or not ex.get('snippet'):
                                    continue
                                try:
                                    src_rel = ex['src'].relative_to(Path.cwd())
                                except Exception:
                                    src_rel = ex['src']
                                pair = (ex.get('id'), str(src_rel))
                                if pair in printed_example_pairs:
                                    continue
                                printed_example_pairs.add(pair)
                                extension = common.determine_extension(ex['src'])
                                tokens = [field.name]
                                snippet = ex.get('snippet','') or ''
                                try:
                                    if 'AUI_DECLARATIVE_FOR' in snippet and 'AUI_DECLARATIVE_FOR' not in tokens:
                                        tokens.append('AUI_DECLARATIVE_FOR')
                                except Exception:
                                    pass
                                hl = _compute_hl_lines(snippet, tokens)
                                hl_attr = f' hl_lines="{hl}"' if hl else ''
                                print(f"\n??? note \"{src_rel}\"", file=fos)
                                print(file=fos)
                                print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                                print(file=fos)
                                print(f"    ```{extension}{hl_attr}", file=fos)
                                for line in ex['snippet'].splitlines():
                                    print(f"    {line}", file=fos)
                                print(f"    ```", file=fos)

            if hasattr(parse_entry, 'methods'):
                methods = [i for i in parse_entry.methods if i.visibility != 'private' and i.doc is not None]
                if methods:
                    print('', file=fos)
                    print('## Public Methods', file=fos)
                    methods_grouped = {}
                    for i in methods:
                        methods_grouped.setdefault(i.name, []).append(i)
                    for name, overloads in sorted(methods_grouped.items(), key=lambda x: x[0] if x[0] != parse_entry.name else '!!!ctor'):
                        full_name = f"{parse_entry.namespaced_name()}::{name}"
                        toc_name = name if name != parse_entry.name else 'constructor'
                        _render_invisible_header(toc=toc_name, id=full_name, on_other_pages=f'{full_name}()')
                        for overload in overloads:
                            print('', file=fos)
                            print('---', file=fos)
                            print('', file=fos)
                            print(f'```cpp', file=fos)
                            if overload.template_clause:
                                print(_format_token_sequence([i[1] for i in overload.template_clause]), file=fos)

                            if overload.modifiers_before:
                                print(_format_token_sequence(overload.modifiers_before), end=' ', file=fos)

                            if overload.return_type: # not a constructor
                                print(f'{overload.return_type} ', end='', file=fos)
                            print(f'{parse_entry.name}::{overload.name}{_format_token_sequence([ i[1] for i in overload.args])}', end='', file=fos)
                            print('', file=fos)
                            print(f'```', file=fos)
                            print(f'', file=fos)

                            doxygen = common.parse_doxygen(overload.doc)

                            # todo add a check for @brief to exist
                            for i in [i for i in doxygen if i[0] == '@brief']:
                                print(i[1], file=fos)
                            print('<dl class="doxygen-dl">', file=fos)
                            params = [i for i in doxygen if i[0] == '@param']
                            if params:
                                print('<dt>Arguments</dt>', file=fos)
                                print('<dd><div style="display: table">', file=fos)
                                for i in params:
                                    print(f'<div style="display: table-row">', file=fos)
                                    argument_name = i[1].split(' ')[0]
                                    print(f'<b style="display: table-cell; padding-right: 6px"><code>{argument_name}</code></b>', file=fos)
                                    print(f'<div style="display: table-cell">{i[1][len(argument_name):]}</div>', file=fos)
                                    print(f'</div>', file=fos)
                                print('</div></dd>', file=fos)
                            for i in doxygen:
                                if i[0] != '@return':
                                    continue
                                print('<dt>Returns</dt>', file=fos)
                                print(f'<dd>{i[1]}</dd>', file=fos)
                            print('</dl>', file=fos)

                            for i in doxygen:
                                if i[0] in ['', '@details']:
                                    print(f'{i[1]}', file=fos)

                            # Examples for this overload: show method-specific examples only.
                            try:
                                method_full = f"{parse_entry.namespaced_name()}::{overload.name}"
                                method_names = [method_full, overload.name]
                                method_exs = _examples_for_symbol_with_snippets(method_names, anchors=[method_full, overload.name], examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None)) or []
                                method_exs = _dedupe_examples_list(method_exs)
                                method_exs = _filter_examples_by_relevance(method_exs, method_names, strict=True)
                                if not method_exs:
                                    fallback_list = _dedupe_examples_list(_examples_for_symbol(method_names, examples_lists=getattr(examples_page, 'examples_lists', None), examples_index=getattr(examples_page, 'examples_index', None)))
                                    # Pre-filter fallback candidates to those that actually contain
                                    # member-access or call patterns for the member token. This
                                    # avoids pulling in unrelated examples that mention the
                                    # token in other contexts.
                                    mem_token = re.escape(overload.name)
                                    member_pattern = re.compile(r"\b" + mem_token + r"\s*\(|\b" + mem_token + r"\s*->|\b" + mem_token + r"\s*\.|AUI_SLOT\(\s*" + mem_token + r"\s*\)")
                                    filtered_fallback = [ex for ex in fallback_list if ex.get('snippet') and member_pattern.search(ex.get('snippet'))]
                                    method_exs = _filter_examples_by_relevance(filtered_fallback, method_names, strict=False)
                                if method_exs:
                                    print('\n**Examples:**\n', file=fos)
                                    for ex in method_exs:
                                        if not ex or 'src' not in ex or not ex.get('snippet'):
                                            continue
                                        try:
                                            src_rel = ex['src'].relative_to(Path.cwd())
                                        except Exception:
                                            src_rel = ex['src']
                                        extension = common.determine_extension(ex['src'])
                                        tokens = [method_full, overload.name]
                                        snippet = ex.get('snippet','') or ''
                                        try:
                                            if 'AUI_DECLARATIVE_FOR' in snippet and 'AUI_DECLARATIVE_FOR' not in tokens:
                                                tokens.append('AUI_DECLARATIVE_FOR')
                                        except Exception:
                                            pass
                                        hl = _compute_hl_lines(snippet, tokens)
                                        hl_attr = f' hl_lines="{hl}"' if hl else ''
                                        print(f"\n??? note \"{src_rel}\"", file=fos)
                                        print(file=fos)
                                        print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                                        print(file=fos)
                                        print(f"    ```{extension}{hl_attr}", file=fos)
                                        for line in ex['snippet'].splitlines():
                                            print(f"    {line}", file=fos)
                                        print(f"    ```", file=fos)
                            except Exception:
                                pass




    with mkdocs_gen_files.open('classes.md', 'w') as f:
        classes_alphabet = { }
        classes = [i for i in cpp_parser.index if isinstance(i, CppClass)]

        for clazz in classes:
            letter = clazz.name[0]
            if letter == 'A' and clazz.name[1].isupper():
                # most classes in AUI start with 'A', so it makes less sense to chunk by 'A'. Instead, we'll use the
                # second letter.
                letter = clazz.name[1]
            letter = letter.upper()
            classes_alphabet.setdefault(letter, []).append(clazz)
        classes_alphabet = sorted(classes_alphabet.items())
        print('<div class="class-index-title">', file=f)
        for letter, _ in classes_alphabet:
            print(f'<a href="#{letter.lower()}">{letter}</a>', file=f)
        print('</div>', file=f)

        print('<div class="class-index" markdown>', file=f)
        for letter, classes2 in classes_alphabet:
            print('<div class="item odd-highlight" markdown>', file=f)
            print(f'<div class="letter" id="{letter.lower()}">{letter}</div>', file=f)
            print('<div class="list" markdown>', file=f)
            for c in sorted(classes2, key=lambda x: x.name):
                if not c.page_url:
                    continue
                print('<div class="entry" markdown>', file=f)
                print(f'[{c.name}]({c.page_url})', file=f)
                # print(f'{c.name}', file=f)
                print('</div>', file=f)
            print('</div>', file=f)
            print('</div>', file=f)
        print('</div>', file=f)

