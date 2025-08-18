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
from pathlib import Path

import mkdocs_gen_files
import pymdownx

from docs.python.generators import cpp_parser
from docs.python.generators.cpp_parser import DoxygenEntry, CppClass

CPP_BRIEF_LINE = re.compile('(\s*\@\w+) ?(.*)')
assert CPP_BRIEF_LINE.match('@brief Test').group(1) == "@brief"
assert CPP_BRIEF_LINE.match('@brief Test').group(2) == "Test"
assert CPP_BRIEF_LINE.match('@brief').group(1) == "@brief"

log = logging.getLogger('mkdocs')

def parse_doxygen(comment):
    output = [['', '']]
    iterator = iter(comment.split('\n'))

    for i in iterator:
        if m := CPP_BRIEF_LINE.match(i):
            section_name = m.group(1)
            output.append([section_name, ''])
            output[-1][1] += m.group(2)
            continue
        output[-1][1] += "\n" + i
    output = [i for i in filter(lambda x: x[1] != '', output)]
    for output_line in output:
        output_line[1] = output_line[1].strip()

    return output


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
            # Arbitrary comment. It may contain group definition or other information.
            doxygen = parse_doxygen(parse_entry.doc)
            for def_group in [i for i in doxygen if i[0] == '@defgroup']:
                m = re.compile(r"(\S+) (.+)").match(def_group[1])
                group_id = m.group(1)
                group_name = m.group(2)

                output = f'{group_id.lower()}.md'
                with mkdocs_gen_files.open(output, 'w') as fos:
                    print(f'# {group_name}\n', file=fos)
                    print(f'<!-- aui:index_alias {group_id} -->', file=fos)
                    for i in [i for i in doxygen if i[0] == '@brief']:
                        print(i[1], file=fos)

                    details = [i for i in doxygen if i[0] in ['@details', '']]
                    if details:
                        print(f'## Detailed Description', file=fos)
                        for i in details:
                            print(i[1], file=fos)
                    print('', file=fos)
                    print('## Related Pages', file=fos)
                    print('', file=fos)

                    print('<div class="grid cards" markdown>', file=fos)
                    print(' ', file=fos)
                    for group_item in cpp_parser.index:
                        if not hasattr(group_item, 'doc'):
                            continue
                        if not hasattr(group_item, 'name'):
                            continue
                        if f"@ingroup {group_id}" not in group_item.doc:
                            continue
                        brief = "\n".join([i[1] for i in parse_doxygen(group_item.doc) if f"@brief" in i[0]])
                        print(f"""
-   __{group_item.name}__

    ---

    {brief}

""", file=fos)

                    print('</div>', file=fos)


                mkdocs_gen_files.set_edit_path(output, '..' / parse_entry.location)
                break

            continue

        slugged_name = parse_entry.namespaced_name().lower().replace('::', '_')
        parse_entry.page_url = f'{slugged_name}.md'
        with mkdocs_gen_files.open(parse_entry.page_url, 'w') as fos:
            print(f'# {parse_entry.namespaced_name()}', file=fos)
            print(f'', file=fos)
            doxygen = parse_doxygen(parse_entry.doc)


            include_dir = parse_entry.location
            while include_dir.name != 'src':
                if include_dir == include_dir.parent:
                    break
                include_dir = include_dir.parent

            module_name = str(include_dir.parent.name).replace('.', '::')

            for i in [i for i in doxygen if i[0] == '@brief']:
                print(i[1], file=fos)

            has_detailed_description = bool([i for i in doxygen if i[0] == '@details'])

            if has_detailed_description:
                print('[More...](#detailed-description)', file=fos)

            print('<table>', file=fos)
            for i in [('Header:', f'<code>#include &lt;{parse_entry.location.relative_to(include_dir)}&gt;</code>'), ('CMake:', f'<code>aui_link(my_target PUBLIC {module_name})</code>')]:
                print(f'<tr><td>{i[0]}</td><td>{i[1]}</td></tr>', file=fos)
            print('</table>', file=fos)

            if hasattr(parse_entry, 'definition'):
                print('## Definition', file=fos)
                print('```cpp', file=fos)
                print(parse_entry.definition, file=fos)
                print('```\n', file=fos)

            if has_detailed_description:
                print('## Detailed Description', file=fos)
                for i in [i for i in doxygen if i[0] == '@details']:
                    print(i[1], file=fos)

            def _render_invisible_header(name):
                # hack: present the header as invisible block. The header will still appear in TOC and
                # can be anchor referenced.
                print(f'<div style="position:absolute;opacity:0" markdown>', file=fos)
                print(f'### {name}', file=fos)
                print(f'</div>', file=fos)

            if hasattr(parse_entry, 'fields'):
                fields = [i for i in parse_entry.fields if i.visibility != 'private' and i.doc is not None]
                if fields:
                    print('## Public fields and Signals', file=fos)
                    for i in sorted(fields, key=lambda x: x.name):
                        print('---', file=fos)
                        _render_invisible_header(i.name)
                        print('```cpp', file=fos)
                        print(f'{i.type_str} {i.name}', file=fos)
                        print('```', file=fos)
                        doxygen = parse_doxygen(i.doc)
                        for i in doxygen:
                            print(i[1], file=fos)


            if hasattr(parse_entry, 'methods'):
                methods = [i for i in parse_entry.methods if i.visibility != 'private' and i.doc is not None]
                if methods:
                    print('', file=fos)
                    print('## Public Methods', file=fos)
                    methods_grouped = {}
                    for i in methods:
                        methods_grouped.setdefault(i.name, []).append(i)
                    for name, overloads in sorted(methods_grouped.items(), key=lambda x: x[0] if x[0] != parse_entry.name else '!!!ctor'):
                        _render_invisible_header(name)
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

                            doxygen = parse_doxygen(overload.doc)

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
            print('<div class="item" markdown>', file=f)
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
