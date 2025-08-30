#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import io
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


def _find_undocumented_aviews():

    name_to_class = {c.name: c for c in cpp_parser.index if hasattr(c, 'name')}

    def get_aview_family(cls, name_to_class, family_set):
        if not hasattr(cls, 'base_classes'):
            return
        family_set.add(cls.name)
        for base in getattr(cls, 'base_classes', []):
            if base in name_to_class:
                get_aview_family(name_to_class[base], name_to_class, family_set)

    def is_derived_from_AView(cls, name_to_class):
        visited = set()
        def check(c):
            if not hasattr(c, 'base_classes'):
                return False
            if c.name == 'AView':
                return True
            for base in getattr(c, 'base_classes', []):
                if base == 'AView':
                    return True
                if base in name_to_class and base not in visited:
                    visited.add(base)
                    if check(name_to_class[base]):
                        return True
            return False
        return check(cls)

    aview_family_names = set()
    for c in cpp_parser.index:
        if not hasattr(c, 'name') or not hasattr(c, 'doc'):
            continue
        if is_derived_from_AView(c, name_to_class):
            get_aview_family(c, name_to_class, aview_family_names)


    for extra in ("AViewContainer", "AViewContainerBase"):
        if extra in name_to_class:
            aview_family_names.add(extra)

    return sorted((name_to_class[n] for n in aview_family_names if n in name_to_class), key=lambda c: c.name.lower())



def define_env(env):
    is_odd = False

    @env.macro
    def group(name):
        """
        Generates docs/useful_views.md with cards for each view class derived from AView (from AView.h), using @brief and page links, matching the style of useful_views_new.md.
        """
        with io.StringIO() as fos:
            nonlocal is_odd
            is_odd_str = "background: var(--md-code-bg-color)" if is_odd else ""
            is_odd = not is_odd
            print(f'<div class="views-grid">', file=fos)
            if is_odd_str:
                print('<div class="odd-overlay"></div>', file=fos)

            views = []
            match name:
                case 'views_other':
                    views = _find_undocumented_aviews()
                case _:
                    for c in cpp_parser.index:
                        if not hasattr(c, 'name') or not hasattr(c, 'doc'):
                            continue
                        if f"@ingroup {name}" in c.doc:
                            if hasattr(c, 'namespaced_name'):
                                slugged_name = c.namespaced_name().lower().replace('::', '_')
                                c.page_url = f'{slugged_name}.md'
                                views.append(c)

            views.sort(key=lambda c: c.name.lower())

            for group_item in views:
                brief = "\n".join([i[1] for i in common.parse_doxygen(group_item.doc) if f"@brief" in i[0]])
                img_match = re.search(r'!\[.*?\]\((.*?)\)', brief)
                if img_match:
                    img_path = f'../{img_match.group(1)}'
                    brief_text = brief[:img_match.start()].rstrip()
                else:
                    img_path = '../imgs/logo_black.svg'
                    brief_text = brief
                url_base = group_item.page_url[:-3] if getattr(group_item, 'page_url', None) else ''
                print(f'''<div class="views-card-outer">
            <a href="../{url_base}">
                <div class="views-card">
                    <img src="{img_path}" alt="{group_item.name} screenshot" onerror="this.src='../imgs/logo_black.svg'">
                </div>
            </a>
            <div class="views-card-title">{group_item.name}</div>
            <div class="views-card-desc">{brief_text}</div>
            <a class="views-card-link" href="../{url_base}">Learn more</a>
        </div>''', file=fos)

            print('\n</div>', file=fos)
            return fos.getvalue()
