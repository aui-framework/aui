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

CPP_COMMENT_LINE = re.compile('\s*\* ?(.*)')
assert CPP_COMMENT_LINE.match('  * Test').group(1) == "Test"

def parse_comment_lines(iterator):
    output = []
    for line in iterator:
        if "*/" in line:
            break
        line = CPP_COMMENT_LINE.match(line).group(1)


        # todo: things like # Platform support {#AFatalException_Platform_support} breaks markdown parser in mkdocs
        line = line.replace('{', '').replace('}', '')

        output.append(line)
    return "\n".join(output)


def gen_pages():
    classes = set()
    for root, dirs, files in os.walk('.'):
        for file in files:
            if not root.startswith('./aui.'):
                continue
            if not file.endswith('.h'):
                continue
            if "3rdparty" in root:
                continue
            with open(Path(root) / file, 'r') as f:
                iterator = iter(f.readlines())
                last_comment_line = None
                for line in iterator:
                    if "/**" in line:
                        last_comment_line = parse_comment_lines(iterator)
                        continue

                    if m := CPP_CLASS_DEF.match(line):
                        if not last_comment_line:
                            continue # non-documented class = non-existing class

                        class_name = m.group(2)
                        classes.add(class_name)
                        with mkdocs_gen_files.open(f'reference/{class_name.lower()}.md', 'w') as fos:
                            print(f'# {class_name}', file=fos)
                            print(f'', file=fos)
                            print(last_comment_line, file=fos)

                    last_comment_line = None



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
                print('<div class="entry">', f'<a href="/reference/{c.lower()}">{c}</a>', '</div>', file=f)
            print('</div>', file=f)
            print('</div>', file=f)
        print('</div>', file=f)

