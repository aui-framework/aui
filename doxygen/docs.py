#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2024 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import os
import shutil
from pathlib import Path

from modules import checks, patching, generators, doxygen_utils, common, toc
from modules.config import CONFIG

def fix_code_whitespaces():
    for root, dirs, files in os.walk("doxygen/out/html"):
        for file in files:
            if not file.endswith(".html"):
                continue
            path = Path(root) / file
            with open(path, 'r') as fis:
                lines = fis.readlines()

            def is_code_line(line):
                return '<div class="line">' in line

            if all([not is_code_line(line) for line in lines]):
                continue

            with open(path, 'w') as fos:
                for line in lines:
                    if is_code_line(line):
                        line = line.replace("  ", "&nbsp;&nbsp;")

                    fos.write(line)

def fix_platform_names():
    for k,v in [('windows', 'Windows'), ('macos', 'macOS'), ('linux', 'Linux'), ('ios', 'iOS'), ('android', 'Android')]:
        patching.patch(target='*.html', matcher=f'{k}-specific', mode=patching.Mode.REPLACE, value=f'{v}-specific')




if __name__ == '__main__':
    checks.is_valid_workdir()

    checks.are_all_in_group(Path("aui.views/src/AUI/ASS/Property"), "ass_properties")
    checks.are_all_in_group(Path("aui.views/src/AUI/ASS/Selector"), "ass_selectors")

    generators.docs_from_tests()
    generators.docs_from_gen()
    generators.docs_examples()
    doxygen_utils.invoke()

    fix_code_whitespaces()
    fix_platform_names()

    patching.patch(target='classes.html', matcher='<div class="contents">', mode=patching.Mode.INSERT_AFTER)
    patching.patch(target='experimental_api.html', matcher='<dl class="reflist">', mode=patching.Mode.REPLACE, value='<dl>')

    # remove useless AUI Framework root element.
    patching.patch(target='navtreedata.js', matcher='[ "AUI Framework", "index.html", [', value='', mode=patching.Mode.DELETE_LINE)
    patching.patch(target='navtreedata.js', matcher=lambda x: x == '  ] ]\n', value='', mode=patching.Mode.DELETE_LINE)
    patching.patch(target='navtree.js', matcher='o.breadcrumbs.unshift', mode=patching.Mode.DELETE_LINE)

    # remove links from groups and instead make them toggle collapse/expand.
    def remove_link_from_group(line):
        # "usergroup0.html", [
        words = line.split(', ')
        if len(words) != 3:
            return False
        if words[2] != '[\n':
            return False
        if '"' not in words[1]:
            return False
        words[1] = "null"
        return ", ".join(words)

    patching.patch(target='navtreedata.js', matcher=remove_link_from_group)

    # patching.patch(target='navtree.js', anchor='(16*level)', value='(16*(level-1))', mode=patching.Mode.REPLACE)
    # patching.patch(target='navtree.js', anchor='(level+1)', value='level', mode=patching.Mode.REPLACE)

    # remove overflow: hidden from body
    try:
        patching.patch(target='resize.js', matcher='$("body")', mode=patching.Mode.DELETE_LINE)
    except:
        pass

    patching.patch(target='*.html', matcher='</div><!-- contents -->', mode=patching.Mode.REPLACE, value=Path('doxygen/footer_inner.html'), unique=True)
    patching.patch(target='doxygen.css', matcher='var(--page-visited-link-color)', mode=patching.Mode.DELETE_LINE)

    ROBOT_NOINDEX = '<meta name="robots" content="noindex">\n'

    for target in ['*_source.html', '*-members.html', 'dir_*.html', '*2intermediate*']:
        patching.patch(target=target, matcher='<meta', mode=patching.Mode.INSERT_AFTER, value=ROBOT_NOINDEX, unique=True)

    toc.run()

    if common.error_flag:
        exit(-1)

    output_dir = Path(CONFIG['output'])
    for file in CONFIG['copy_to_output']:
        shutil.copytree(file, output_dir / Path(file).name, dirs_exist_ok=True)
        pass
