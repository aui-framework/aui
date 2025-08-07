#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import os
import re
from pathlib import Path

import mkdocs_gen_files

from docs.python import regexes

CPP_CLASS_DEF = re.compile('class( API_\S+)? ([a-zA-Z0-9_$]+)')
assert CPP_CLASS_DEF.match('class Test').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test;').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test {').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test: Base {').group(2) == "Test"

def gen_pages():
    classes = []
    for root, dirs, files in os.walk('.'):
        for file in files:
            if not root.startswith('./aui.'):
                continue
            if not file.endswith('.h'):
                continue
            if "3rdparty" in root:
                continue
            with open(Path(root) / file, 'r') as f:
                for line in f.readlines():
                    if m := CPP_CLASS_DEF.match(line):
                        classes.append(m.group(2))


    with mkdocs_gen_files.open('classes.md', 'w') as f:
        classes_alphabet = { }

        for i in classes:
            letter = i[0]
            if letter == 'A' and i[1].isupper():
                # most classes in AUI start with 'A', so it makes less sense to chunk by 'A'. Instead, we'll use the
                # second letter.
                letter = i[1]
            letter = letter.upper()
            classes_alphabet.setdefault(letter, []).append(i)
        classes_alphabet = sorted(classes_alphabet.items())
        print('<div class="class-index-title">', file=f)
        for letter, _ in classes_alphabet:
            print(f'<a href="#{letter.lower()}">{letter}</a>', file=f)
        print('</div>', file=f)

        print('<div class="class-index">', file=f)
        for letter, classes2 in classes_alphabet:
            print('<div class="item">', file=f)
            print(f'<div class="letter" id="{letter.lower()}">{letter}</div>', file=f)
            print('<div class="list">', file=f)
            for c in sorted(classes2):
                print('<div class="entry">', c, '</div>', file=f)
            print('</div>', file=f)
            print('</div>', file=f)
        print('</div>', file=f)

