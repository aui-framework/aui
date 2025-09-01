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

from docs.python.generators import cpp_parser, common, group_page, examples_page
from docs.python.generators.cpp_parser import DoxygenEntry, CppClass
from pathlib import Path

log = logging.getLogger('mkdocs')


def _examples_for_symbol_with_snippets(names: list[str]):
    """Search examples index and lists for occurrences of provided names.

    Returns list of dicts with keys: title, id, description, src (Path), snippet, category
    """
    results = []
    try:
        index = examples_page.examples_index
    except Exception:
        index = None

    seen = set()

    if index is not None:
        for name in names:
            if not name:
                continue
            for ex in index.get(name, []):
                key = (ex.get('id'))
                if key in seen:
                    continue
                # find the specific src that contains the token
                for src in ex.get('srcs', []):
                    try:
                        text = src.read_text(encoding='utf-8', errors='ignore')
                    except Exception:
                        continue
                    if re.search(r"\b" + re.escape(name) + r"\b", text):
                        # build snippet
                        lines = text.splitlines()
                        m = re.search(r"\b" + re.escape(name) + r"\b", text)
                        pos = m.start()
                        cum = 0
                        line_idx = 0
                        for i, l in enumerate(lines):
                            if pos <= cum + len(l):
                                line_idx = i
                                break
                            cum += len(l) + 1
                        start = max(0, line_idx - 2)
                        end = min(len(lines), line_idx + 3)
                        snippet = '\n'.join(lines[start:end])
                        results.append({
                            'title': ex['title'],
                            'id': ex['id'],
                            'description': ex['description'],
                            'src': Path(src),
                            'snippet': snippet,
                        })
                        seen.add(key)
                        break
    # fallback: scan lists manually
    try:
        lists = examples_page.examples_lists
    except Exception:
        lists = {}

    for category, items in lists.items():
        for ex in items:
            for src in ex.get('srcs', []):
                try:
                    text = src.read_text(encoding='utf-8', errors='ignore')
                except Exception:
                    continue
                for name in names:
                    if not name:
                        continue
                    m = re.search(r"\b" + re.escape(name) + r"\b", text)
                    if not m:
                        continue
                    key = (ex.get('id'), str(src))
                    if key in seen:
                        continue
                    lines = text.splitlines()
                    pos = m.start()
                    cum = 0
                    line_idx = 0
                    for i, l in enumerate(lines):
                        if pos <= cum + len(l):
                            line_idx = i
                            break
                        cum += len(l) + 1
                    start = max(0, line_idx - 2)
                    end = min(len(lines), line_idx + 3)
                    snippet = '\n'.join(lines[start:end])
                    results.append({
                        'title': ex['title'],
                        'id': ex['id'],
                        'description': ex['description'],
                        'src': Path(src),
                        'snippet': snippet,
                        'category': category,
                    })
                    seen.add(key)
    return results


def _format_token_sequence(tokens: list[str]):
    output = " ".join(tokens)
    for i in [",", "&&", ">", "&", "*"]:
        output = output.replace(f" {i} ", f"{i} ")
    for i in "()<[]:":
        output = output.replace(f"{i} ", i)
        output = output.replace(f" {i}", i)
    return output


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
            print(f'# {parse_entry.namespaced_name()}', file=fos)
            print(f'', file=fos)
            doxygen = common.parse_doxygen(parse_entry.doc)
            page_examples: list[dict] = []

            def _examples_for_symbol(names: list[str]):
                try:
                    index = examples_page.examples_index
                except Exception:
                    index = None

                merged = []
                seen = set()

                if index is not None:
                    for name in names:
                        if not name:
                            continue
                        for ex in index.get(name, []):
                            key = ex.get('id')
                            if key in seen:
                                continue
                            seen.add(key)
                            merged.append(ex)
                    if merged:
                        return merged

                try:
                    lists = examples_page.examples_lists
                except Exception:
                    return []

                for category, items in lists.items():
                    for ex in items:
                        for src in ex.get('srcs', []):
                            try:
                                text = src.read_text(encoding='utf-8', errors='ignore')
                            except Exception:
                                continue
                            for name in names:
                                if not name:
                                    continue
                                if re.search(r"\b" + re.escape(name) + r"\b", text):
                                    key = (ex['id'], ex['title'])
                                    if key in seen:
                                        break
                                    seen.add(key)
                                    merged.append(ex)
                                    break
                            else:
                                continue
                            break
                return merged


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
                    exs = _examples_for_symbol_with_snippets(names_to_search)
                    if exs:
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
                print('\n## Examples', file=fos)
                for ex in page_examples:
                    try:
                        src_rel = ex['src'].relative_to(Path.cwd())
                    except Exception:
                        src_rel = ex['src']
                    extension = common.determine_extension(ex['src'])
                    print(f"??? note \"{src_rel}\"", file=fos)
                    print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                    print(f"    ```{extension}", file=fos)
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
                class_examples = _examples_for_symbol(top_names) or []
            except Exception:
                class_examples = []
                pass

            # For classes: print class-level examples once (used as fallback reference).
            if isinstance(parse_entry, CppClass) and class_examples:
                try:
                    exs = _examples_for_symbol_with_snippets([parse_entry.namespaced_name(), parse_entry.name])
                    if exs:
                        print('\n## Examples', file=fos)
                        for ex in exs:
                            try:
                                src_rel = ex['src'].relative_to(Path.cwd())
                            except Exception:
                                src_rel = ex['src']
                            extension = common.determine_extension(ex['src'])
                            print(f"\n??? note \"{src_rel}\"", file=fos)
                            print(file=fos)
                            print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                            print(file=fos)
                            print(f"    ```{extension}", file=fos)
                            for line in ex['snippet'].splitlines():
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
                    for type_entry in types:
                        full_name = f"{parse_entry.namespaced_name()}::{type_entry.name}"
                        _render_invisible_header(toc=type_entry.name, id=full_name, on_other_pages=full_name)
                        print('', file=fos)
                        print('---', file=fos)
                        print('', file=fos)
                        print(f'`{type_entry.generic_kind} {parse_entry.namespaced_name()}::{type_entry.name}`', file=fos)
                        print(f'', file=fos)

                        doxygen = common.parse_doxygen(type_entry.doc)

                        # todo add a check for @brief to exist
                        for i in [i for i in doxygen if i[0] == '@brief']:
                            print(i[1], file=fos)

                        for i in doxygen:
                            if i[0] in ['', '@details']:
                                print(f'{i[1]}', file=fos)

                        # Examples for nested types
                        names_to_search = [full_name, type_entry.name]
                        exs = _examples_for_symbol_with_snippets(names_to_search)
                        if exs:
                            print('\n## Examples', file=fos)
                            for ex in exs:
                                try:
                                    src_rel = ex['src'].relative_to(Path.cwd())
                                except Exception:
                                    src_rel = ex['src']
                                extension = common.determine_extension(ex['src'])
                                print(f"\n??? note \"{src_rel}\"", file=fos)
                                print(file=fos)
                                print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                                print(file=fos)
                                print(f"    ```{extension}", file=fos)
                                for line in ex['snippet'].splitlines():
                                    print(f"    {line}", file=fos)
                                print(f"    ```", file=fos)

                        # enums - see APath::DefaultPath
                        if hasattr(type_entry, 'enum_values'):
                            print('<table markdown>', file=fos)
                            print('<tr markdown>', file=fos)
                            print('<th>Constant</th>', file=fos)
                            print('<th>Description</th>', file=fos)
                            print('</tr>', file=fos)
                            for v in type_entry.enum_values:
                                print('<tr markdown>', file=fos)
                                print('<td markdown>', file=fos)
                                print(f'`#!cpp {type_entry.name}::{v[0]}`', file=fos)
                                print('</td>', file=fos)
                                print('<td markdown>', file=fos)
                                for i in common.parse_doxygen(v[1]):
                                    print(i[1], file=fos)
                                print('</td>', file=fos)
                                print('</tr>', file=fos)

                            print('</table>', file=fos)

                        # nested struct - see AUpdater::InstallCmdline
                        if hasattr(type_entry, 'fields'):
                            if type_entry.fields:
                                print('<table markdown>', file=fos)
                                print('<tr markdown>', file=fos)
                                print('<th>Field</th>', file=fos)
                                print('<th>Description</th>', file=fos)
                                print('</tr>', file=fos)
                                for v in type_entry.fields:
                                    print('<tr markdown>', file=fos)
                                    print('<td markdown>', file=fos)
                                    print(f'`#!cpp {v.type_str} {v.name}`', file=fos)
                                    print('</td>', file=fos)
                                    print('<td markdown>', file=fos)
                                    for i in common.parse_doxygen(v.doc):
                                        print(i[1], file=fos)
                                    print('</td>', file=fos)
                                    print('</tr>', file=fos)

                                print('</table>', file=fos)
                            else:
                                print('\n\n_Empty structure._', file=fos)


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
                        exs = _examples_for_symbol_with_snippets(names_to_search)
                        if exs:
                            print('## Examples', file=fos)
                            for ex in exs:
                                try:
                                    src_rel = ex['src'].relative_to(Path.cwd())
                                except Exception:
                                    src_rel = ex['src']
                                extension = common.determine_extension(ex['src'])
                                print(f"\n??? note \"{src_rel}\"", file=fos)
                                print(file=fos)
                                print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                                print(file=fos)
                                print(f"    ```{extension}", file=fos)
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
                        _render_invisible_header(toc=f"{name}", id=full_name, on_other_pages=f'{full_name}()')
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
                                method_exs = _examples_for_symbol_with_snippets(method_names) or []
                                if method_exs:
                                    print('\n## Examples', file=fos)
                                    for ex in method_exs:
                                        try:
                                            src_rel = ex['src'].relative_to(Path.cwd())
                                        except Exception:
                                            src_rel = ex['src']
                                        extension = common.determine_extension(ex['src'])
                                        print(f"\n??? note \"{src_rel}\"", file=fos)
                                        print(file=fos)
                                        print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                                        print(file=fos)
                                        print(f"    ```{extension}", file=fos)
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

