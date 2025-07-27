#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

import re

# Match any path containing aui.<module>/tests (or aui.<module>\tests)
DIR = re.compile(
    r'.*(aui\.[^/\\]+)[/\\]tests(?:[/\\]|$)',
    re.IGNORECASE
)
assert DIR.match("/home/aui.views/tests")
assert DIR.match(r"C:\projects\aui.core\tests\example.cpp")
assert not DIR.match("/home/aui.views/src")

# Match lines like: // AUI_DOCS_OUTPUT: path/to/output
# Allow optional spaces, CRLF or LF, and find it anywhere in the file
AUI_DOCS_OUTPUT = re.compile(
    r'^\s*//\s*AUI_DOCS_OUTPUT:\s*(.+)$',
    re.MULTILINE
)
assert AUI_DOCS_OUTPUT.search("// AUI_DOCS_OUTPUT: foo/bar.md\n")
assert AUI_DOCS_OUTPUT.search("   //AUI_DOCS_OUTPUT: C:\\out\\file.txt\r\n")

COMMENT = re.compile(r'\s*// ?(.*)\n?$')
assert COMMENT.match("   // AUI_DOCS_CODE_BEGIN\n")

# TEST_F(UIDataBindingTest, AProperty) { // HEADER_H2
TESTCASE_HEADER_H1 = re.compile(r'TEST(_F)?\((.+), (.+)\) ?\{ *// *HEADER_H1')
TESTCASE_HEADER_H2 = re.compile(r'TEST(_F)?\((.+), (.+)\) ?\{ *// *HEADER_H2')

INGROUP = re.compile(r'.*([@\\]ingroup ?\w*).*')
assert INGROUP.match("   * @ingroup")
assert INGROUP.match("   * @ingroup sadkal")

INCLUDE = re.compile("#include [\"<](.+)[\">].*")
assert INCLUDE.match("#include <SOME.h> ikf").group(1) == "SOME.h"
assert INCLUDE.match("#include \"SOME.h\" ikf").group(1) == "SOME.h"

LOCATION_FILE = re.compile('.*<location file=\"(.+)\" line=\"([0-9]+)\" .*/>')
assert LOCATION_FILE.match('    <location file="aui.core/src/AUI/Common/AProperty.h" line="145" column="1" bodyfile="aui.core/src/AUI/Common/AProperty.h" bodystart="145" bodyend="279"/>')

AUI_EXAMPLE = re.compile('@auiexample{(.+)}')
assert AUI_EXAMPLE.match("@auiexample{app}\n").group(1) == "app"
