#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import re

import mkdocs_gen_files

from docs.python.generators import cpp_parser, common
from docs.python.generators.cpp_parser import DoxygenEntry


def handle_doxygen_entry(parse_entry: DoxygenEntry):
    # Arbitrary comment. It may contain group definition or other information.
    doxygen = common.parse_doxygen(parse_entry.doc)
    for def_group in [i for i in doxygen if i[0] == '@defgroup']:
        m = re.compile(r"(\S+) (.+)").match(def_group[1])
        group_id = m.group(1)
        group_name = m.group(2)

        output = f'{group_id.lower()}.md'
        with mkdocs_gen_files.open(output, 'w') as fos:
            print(f'# {group_name}\n', file=fos)
            print(f'<!-- aui:index_alias {group_id} -->', file=fos)
            for type_entry in [i for i in doxygen if i[0] == '@brief']:
                print(type_entry[1], file=fos)

            match group_id:
                case 'useful_views':
                    _generate_useful_views_group_page(doxygen, fos)
                case _:
                    _generate_regular_group_page(doxygen, fos, group_id)

        mkdocs_gen_files.set_edit_path(output, '..' / parse_entry.location)
        break

def _generate_regular_group_page(doxygen, fos, group_id):
    details = [i for i in doxygen if i[0] in ['@details', '']]
    if details:
        print(f'## Detailed Description', file=fos)
        for type_entry in details:
            print(type_entry[1], file=fos)
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
        brief = "\n".join([i[1] for i in common.parse_doxygen(group_item.doc) if f"@brief" in i[0]])
        print(f"""
-   __{group_item.namespaced_name()}__

---

{brief}

""", file=fos)

    print('</div>', file=fos)


def _generate_useful_views_group_page(doxygen, fos):
    """
    Generates docs/useful_views.md with cards for each view class derived from AView (from AView.h), using @brief and page links, matching the style of useful_views_new.md.
    """
    print('<div class="views-grid">', file=fos)
    for group_item in cpp_parser.index:
        if not hasattr(group_item, 'doc'):
            continue
        if not hasattr(group_item, 'name'):
            continue
        if f"@ingroup useful_views" not in group_item.doc:
            continue
        brief = "\n".join([i[1] for i in common.parse_doxygen(group_item.doc) if f"@brief" in i[0]])
        page_url = getattr(group_item, 'page_url', None)
        if not page_url:
            page_url = group_item.namespaced_name().lower().replace('::', '_') + '.md'
        img_path = f'../imgs/Views/{group_item.name}.png'
        print(f'''<div class="views-card-outer">
    <div class="views-card">
        <a href="{page_url}">
            <img src="{img_path}" alt="{group_item.name} screenshot" onerror="this.src='../imgs/logo_black.svg'">
        </a>
    </div>
    <div class="views-card-title">{group_item.name}</div>
    <div class="views-card-desc">{brief}</div>
    <a class="views-card-link" href="{page_url}">Learn more</a>
</div>''', file=fos)
    print('\n</div>', file=fos)
