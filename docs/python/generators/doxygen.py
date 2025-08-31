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

from docs.python.generators import cpp_parser, common, group_page
from docs.python.generators.cpp_parser import DoxygenEntry, CppClass

log = logging.getLogger('mkdocs')


def _format_token_sequence(tokens: list[str]):
    output = " ".join(tokens)
    for i in [",", "&&", ">", "&", "*"]:
        output = output.replace(f" {i} ", f"{i} ")
    for i in "()<[]:":
        output = output.replace(f"{i} ", i)
        output = output.replace(f" {i}", i)
    return output


def embed_doc(nested, fos):
    doxygen = common.parse_doxygen(nested.doc)

    # todo add a check for @brief to exist
    for i in [i for i in doxygen if i[0] == '@brief']:
        print(i[1], file=fos)

    for i in doxygen:
        if i[0] in ['', '@details']:
            print(f'{i[1]}', file=fos)

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

                            if m := re.match('title: (\w+)', j):
                                # hack: use the overrided name if it exists
                                parse_entry.overview_page_title = m.group(1)

                            if j == '---':
                                print('', file=fos)
                                return
            _parse_page_metadata()

            print(f'# {parse_entry.namespaced_name()}', file=fos)
            print(f'', file=fos)
            doxygen = common.parse_doxygen(parse_entry.doc)


            include_dir = parse_entry.location
            while include_dir.name != 'src':
                if include_dir == include_dir.parent:
                    break
                include_dir = include_dir.parent

            module_name = str(include_dir.parent.name).replace('.', '::')

            for type_entry in [i for i in doxygen if i[0] == '@brief']:
                print(type_entry[1], file=fos)

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
                        embed_doc(nested, fos)

            if hasattr(parse_entry, 'fields'):
                fields = [i for i in parse_entry.fields if i.visibility != 'private' and i.doc is not None]
                if fields:
                    print('## Public fields and Signals', file=fos)
                    for i in sorted(fields, key=lambda x: x.name):
                        print('---', file=fos)
                        full_name = f"{parse_entry.namespaced_name()}::{i.name}"
                        _render_invisible_header(toc=i.name, id=full_name, on_other_pages=full_name)
                        print(f'`#!cpp {i.type_str} {i.name}`\n\n', file=fos)
                        doxygen = common.parse_doxygen(i.doc)
                        for i in doxygen:
                            if i[0] != '@brief':
                                continue
                            print(i[1], file=fos)
                        print('', file=fos)
                        for i in doxygen:
                            if i[0] == '@brief':
                                continue
                            print(i[1], file=fos)
                        print(f'\n', file=fos)


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

