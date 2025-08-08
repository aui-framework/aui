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

from docs.python import regexes, cpp_tokenizer

CPP_CLASS_DEF = re.compile('class( API_\S+)? ([a-zA-Z0-9_$]+)')
assert CPP_CLASS_DEF.match('class Test').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test;').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test {').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test: Base {').group(2) == "Test"

CPP_COMMENT_LINE = re.compile('\s*\* ?(.*)')
assert CPP_COMMENT_LINE.match('  * Test').group(1) == "Test"

CPP_BRIEF_LINE = re.compile('(\s*\@\w+) ?(.*)')
assert CPP_BRIEF_LINE.match('@brief Test').group(1) == "@brief"
assert CPP_BRIEF_LINE.match('@brief Test').group(2) == "Test"
assert CPP_BRIEF_LINE.match('@brief').group(1) == "@brief"

def parse_comment_lines(iterator):
    output = []
    for line in iterator:
        if "/*" in line:
            continue
        if "*/" in line:
            break
        line = CPP_COMMENT_LINE.match(line).group(1)


        # todo: things like # Platform support {#AFatalException_Platform_support} breaks markdown parser in mkdocs
        line = line.replace('{', '').replace('}', '')

        output.append(line.strip())
    return "\n".join(output)

def parse_doxygen(comment):
    output = [['', '']]
    iterator = iter(comment.split('\n'))

    for i in iterator:
        if m := CPP_BRIEF_LINE.match(i):
            section_name = m.group(1)
            output.append([section_name, ''])
            output[-1][1] += m.group(2)
            continue
        output[-1][1] += " " + i
    output = [i for i in filter(lambda x: x[1] != '', output)]
    for output_line in output:
        output_line[1] = output_line[1].strip()

    return output

class CppClass:
    def __init__(self):
        self.name = None
        self.doc = None
        self.methods = []
        self.fields = []

def _parse(input: str):
    tokens = cpp_tokenizer.tokenize(input)
    iterator = iter(tokens)
    last_doc = None

    for token in iterator:
        def _parse_comment():
            nonlocal last_doc
            nonlocal token
            if token[0] == cpp_tokenizer.Type.COMMENT:
                if not "/**" in token[1]:
                    return True
                last_doc = parse_comment_lines(token[1].split('\n'))
                return True
            return False

        def _consume_comment():
            nonlocal last_doc
            doc = last_doc
            last_doc = None
            return doc

        def _skip_special_clause():
            nonlocal token
            nonlocal iterator
            assert token[0] == cpp_tokenizer.Type.SPECIAL_OPEN
            special_open = 1
            out = [ token ]
            while special_open > 0:
                token = next(iterator)
                out.append(token)
                match token[0]:
                    case cpp_tokenizer.Type.SPECIAL_OPEN:
                        special_open += 1
                    case cpp_tokenizer.Type.SPECIAL_CLOSE:
                        special_open -= 1
            return out

        def _parse_type():
            nonlocal token
            nonlocal iterator
            out = token[1]
            while token[1] in ['const', 'volatile']:
                token = next(iterator)
                out += " " + token[1]

            token = next(iterator)

            if token[0] == cpp_tokenizer.Type.SPECIAL_OPEN and not token[1] == '(':
                out += "".join([i[1] for i in _skip_special_clause()])
                token = next(iterator)
            while token[1] in '&*':
                out += token[1]
                token = next(iterator)
            return out
        def _parse_class_body(clazz: CppClass):
            nonlocal iterator
            nonlocal token
            assert token[1] == '{'

            visibility = 'private'

            for token in iterator:
                if _parse_comment():
                    continue
                if token[1] == '}':
                    break
                if token[1] in ['public', 'private', 'protected']:
                    visibility = token[1]
                    assert next(iterator)[1] == ':'
                    continue
                if token[0] == cpp_tokenizer.Type.IDENTIFIER:
                    if token[1] == 'enum':
                        while token[1] != ';':
                            token = next(iterator)
                        continue

                    while True:
                        type_str = _parse_type()
                        if type_str in ["explicit", "static"]:
                            continue
                        break
                    assert token[0] == cpp_tokenizer.Type.IDENTIFIER
                    name = token[1]
                    token = next(iterator)
                    if token[1] == ';':
                        clazz.fields.append((type_str, name, _consume_comment()))
                        continue
                    if token[1] == '(':
                        clazz.methods.append((type_str, name, _consume_comment()))
                        _skip_special_clause()
                        token = next(iterator)
                    if token[1] == ';':
                        continue
                    if token[1] == '{':
                        _skip_special_clause()


        if _parse_comment():
            continue

        if token == (cpp_tokenizer.Type.IDENTIFIER, 'class'):
            token = next(iterator)
            if token[1].startswith("AUI_"):
                # export macro, ignore
                token = next(iterator)
            assert token[0] == cpp_tokenizer.Type.IDENTIFIER
            clazz = CppClass()
            clazz.name = token[1]
            clazz.doc = _consume_comment()

            while token[1] not in '{;':
                token = next(iterator)
            if token[1] == '{':
                _parse_class_body(clazz)

            if clazz.doc:
                yield clazz




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
            full_path = Path(root) / file
            include_dir = full_path
            while include_dir.name != 'src':
                if include_dir == include_dir.parent:
                    break
                include_dir = include_dir.parent

            contents = _parse(full_path.read_text())

            for i in contents:
                class_name = i.name
                classes.add(class_name)
                with mkdocs_gen_files.open(f'reference/{class_name.lower()}.md', 'w') as fos:
                    print(f'# {class_name}', file=fos)
                    print(f'', file=fos)
                    doxygen = parse_doxygen(i.doc)

                    module_name = str(include_dir.parent.name).replace('.', '::')

                    for i in [i for i in doxygen if i[0] == '@brief']:
                        print(i[1], file=fos)

                    has_detailed_description = bool([i for i in doxygen if i[0] == '@details'])

                    if has_detailed_description:
                        print('[More...](#detailed-description)', file=fos)

                    print('<table>', file=fos)
                    for i in [('Header:', f'<code>#include &lt;{full_path.relative_to(include_dir)}&gt;</code>'), ('CMake:', f'<code>aui_link(my_target PUBLIC {module_name})</code>')]:
                        print(f'<tr><td>{i[0]}</td><td>{i[1]}</td></tr>', file=fos)
                    print('</table>', file=fos)

                    if has_detailed_description:
                        print('## Detailed Description', file=fos)
                        for i in [i for i in doxygen if i[0] == '@details']:
                            print(i[1], file=fos)



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

def test_parse_comment_lines():
    iterator = iter(Path('test_data/AString.h').read_text().splitlines())
    for i in iterator:
        if "/**" in iterator:
            break
    assert parse_comment_lines(iterator) =="""@brief Represents a Unicode character string.
@ingroup core
@details
AString stores a string of 16-bit chars, where each char corresponds to one UTF-16 code unit. Unicode characters with
code values above 65535 are stored using two consecutive chars.

Unicode is an international standard that supports most of the writing systems in use today."""

def test_parse_class1():
    assert [i for i in _parse("""
class Test;
    """)] == []


def test_parse_class2():
    assert [i for i in _parse("""
class Test;
    """)] == []

def test_parse_class3():
    assert [i for i in _parse("""
class Test {};
    """)] == []

def test_parse_class4():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {};
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'

def test_parse_class5():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    void hello();
};
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'
    assert clazz.methods == [('void', 'hello', None)]

def test_parse_class6():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    void hello();
};
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'
    assert clazz.methods == [('void', 'hello', '@brief Hello')]

def test_parse_class7():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    void hello();
    
    /**
     * @brief World
     */
    int world();
};
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'
    assert clazz.methods == [
        ('void', 'hello', '@brief Hello'),
        ('int', 'world', '@brief World'),
    ]

def test_parse_class8():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    void hello() {
        if (test) {
        }
    }
    
    /**
     * @brief World
     */
    int world();
};
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'
    assert clazz.methods == [
        ('void', 'hello', '@brief Hello'),
        ('int', 'world', '@brief World'),
    ]


def test_parse_class_enum():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    enum class Kek : uint8_t { VAL };

    /**
     * @brief Hello
     */
    void hello() {
        if (test) {
        }
    }
    
    /**
     * @brief World
     */
    int world();
};
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'
    assert clazz.methods == [
        ('void', 'hello', '@brief Hello'),
        ('int', 'world', '@brief World'),
    ]

def test_parse_class_complex_return1():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    _<Test> hello() {
        if (test) {
        }
    }
    
    /**
     * @brief World
     */
    int world();
};
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'
    assert clazz.methods == [
        ('_<Test>', 'hello', '@brief Hello'),
        ('int', 'world', '@brief World'),
    ]

def test_parse_class_complex_return_cref():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    const _<Test>& hello() {
        if (test) {
        }
    }
    
    /**
     * @brief World
     */
    int world();
};
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'
    assert clazz.methods == [
        ('const _<Test>&', 'hello', '@brief Hello'),
        ('int', 'world', '@brief World'),
    ]


def test_parse_class_complex_return_args():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    _<Test> hello(_<Test> test) {
        if (test) {
        }
    }
    
    /**
     * @brief World
     */
    int world();
};
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'
    assert clazz.methods == [
        ('_<Test>', 'hello', '@brief Hello'),
        ('int', 'world', '@brief World'),
    ]
