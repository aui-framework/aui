#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

import re

DIR = re.compile(r'.*(aui\..+)/tests')
assert DIR.match("/home/aui.views/tests")
assert not DIR.match("/home/aui.views/src")

COMMENT = re.compile(r'\s*// ?(.*)\n?$')
assert COMMENT.match("   // AUI_DOCS_CODE_BEGIN\n")

# TEST_F(UIDataBindingTest, AProperty) { // HEADER_H3
TESTCASE_HEADER_H1 = re.compile(r'TEST(_F)?\((.+), (.+)\) ?\{ *// *HEADER_H1')
TESTCASE_HEADER_H2 = re.compile(r'TEST(_F)?\((.+), (.+)\) ?\{ *// *HEADER_H2')
TESTCASE_HEADER_H3 = re.compile(r'TEST(_F)?\((.+), (.+)\) ?\{ *// *HEADER_H3')

INGROUP = re.compile(r'.*([@\\]ingroup ?\w*).*')
assert INGROUP.match("   * @ingroup")
assert INGROUP.match("   * @ingroup sadkal")

INCLUDE = re.compile(r"#include [\"<](.+)[\">].*")
assert INCLUDE.match("#include <SOME.h> ikf").group(1) == "SOME.h"
assert INCLUDE.match("#include \"SOME.h\" ikf").group(1) == "SOME.h"

LOCATION_FILE = re.compile(r'.*<location file=\"(.+)\" line=\"([0-9]+)\" .*/>')
assert LOCATION_FILE.match('    <location file="aui.core/src/AUI/Common/AProperty.h" line="145" column="1" bodyfile="aui.core/src/AUI/Common/AProperty.h" bodystart="145" bodyend="279"/>')

AUI_EXAMPLE = re.compile(r'<!-- aui:example (.+) -->')
assert AUI_EXAMPLE.match("<!-- aui:example app -->").group(1) == "app"

PAGE_TITLE = re.compile(r'^# (.+)')
assert PAGE_TITLE.match('# AUI Framework').group(1) == "AUI Framework"

HEADING_ANCHOR = re.compile(r'[#]{1,6} (.+) (\{ #(\S+) ?(\S*) \})')
assert HEADING_ANCHOR.match('## AUI Framework { #aui }').group(1) == "AUI Framework"
assert HEADING_ANCHOR.match('## AUI Framework { #aui }').group(3) == "aui"
assert HEADING_ANCHOR.match('## AUI Framework { #aui aboba::aboba() }').group(4) == "aboba::aboba()"

INDEX_ALIAS = re.compile(r'<!-- aui:index_alias (.+) -->')
assert INDEX_ALIAS.match('<!-- aui:index_alias AUI_DECLARATIVE_FOR -->').group(1) == "AUI_DECLARATIVE_FOR"

MACRO_DEFINE = re.compile(r'#define ([\w\d_$]+)')
assert MACRO_DEFINE.match('#define AUI_HELLO()').group(1) == "AUI_HELLO"

HREF_INJECT = re.compile(r'(<[^>]*>|[\w\'_\.:]+(\(\))?|.)', flags=re.S)
assert HREF_INJECT.match('AProcess::self()->hello').group(1) == 'AProcess::self()'
assert HREF_INJECT.match('AProcess::self->hello').group(1) == 'AProcess::self'

CPP_BRIEF_LINE = re.compile(r'(\s*\@\w+) ?(.*)')
assert CPP_BRIEF_LINE.match('@brief Test').group(1) == "@brief"
assert CPP_BRIEF_LINE.match('@brief Test').group(2) == "Test"
assert CPP_BRIEF_LINE.match('@brief').group(1) == "@brief"
