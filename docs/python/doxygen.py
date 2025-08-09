#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import logging
import os
import re
from pathlib import Path

import mkdocs_gen_files
from pygments.lexers.csound import newline

from docs.python import regexes, cpp_tokenizer

CPP_CLASS_DEF = re.compile('class( API_\S+)? ([a-zA-Z0-9_$]+)')
assert CPP_CLASS_DEF.match('class Test').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test;').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test {').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test: Base {').group(2) == "Test"

CPP_COMMENT_LINE = re.compile('\s*\* ?(.*)')
assert CPP_COMMENT_LINE.match('  * Test').group(1) == "Test"
assert CPP_COMMENT_LINE.match('  *  Test').group(1) == " Test"

CPP_BRIEF_LINE = re.compile('(\s*\@\w+) ?(.*)')
assert CPP_BRIEF_LINE.match('@brief Test').group(1) == "@brief"
assert CPP_BRIEF_LINE.match('@brief Test').group(2) == "Test"
assert CPP_BRIEF_LINE.match('@brief').group(1) == "@brief"

log = logging.getLogger('doxygen')

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

        output.append(line)
    return "\n".join(output)

def parse_doxygen(comment):
    output = [['', '']]
    iterator = iter(comment.split('\n'))

    details_found = False
    for i in iterator:
        if not details_found:
            if m := CPP_BRIEF_LINE.match(i):
                section_name = m.group(1)
                output.append([section_name, ''])
                output[-1][1] += m.group(2)
                if section_name == '@details':
                    details_found = True
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

class CppFunction:
    def __init__(self, name, return_type = None, doc = None, args = None, visibility = 'public', template_clause = None, modifiers_before = None):
        self.name = name
        self.return_type = return_type
        self.doc = doc
        self.args = args
        self.visibility = visibility
        self.template_clause = template_clause
        self.modifiers_before = modifiers_before


    def tuple(self):
        return self.name, self.return_type, self.doc

    def __eq__(self, other):
        return self.tuple() == other.tuple()

    def __str__(self):
        return f'CppFunction{self.tuple()}'

class CppClass:
    def __init__(self):
        self.name = None
        self.doc = None
        self.methods = []
        self.fields = []

    def tuple(self):
        return self.name, self.doc, self.methods, self.fields

    def __eq__(self, other):
        return self.tuple() == other.tuple()
    def __str__(self):
        return f'CppClass{self.tuple()}'

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

        def _skip_special_clause(open = [ cpp_tokenizer.Type.GENERIC_OPEN ], close = [ cpp_tokenizer.Type.GENERIC_CLOSE ]):
            nonlocal token
            nonlocal iterator
            assert token[0] in open
            special_open = 1
            out = [ token ]
            while special_open > 0:
                token = next(iterator)
                out.append(token)
                if token[0] in open:
                    special_open += 1
                elif token[0] in close:
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

            # handle namespace types
            while token[1] == '::':
                out += token[1]
                token = next(iterator)
                out += token[1]
                token = next(iterator)

            is_decltype = 'decltype' in out
            is_template_wizardy = token[0] in [ cpp_tokenizer.Type.GENERIC_OPEN, cpp_tokenizer.Type.GENERIC_OPEN2 ] and not token[1] == '('

            if is_decltype or is_template_wizardy:
                out += "".join([i[1] for i in _skip_special_clause([ cpp_tokenizer.Type.GENERIC_OPEN, cpp_tokenizer.Type.GENERIC_OPEN2 ], [ cpp_tokenizer.Type.GENERIC_CLOSE, cpp_tokenizer.Type.GENERIC_CLOSE2 ])])
                token = next(iterator)
            while token[1] in '&*':
                out += token[1]
                token = next(iterator)
            return out
        def _parse_class_body(clazz: CppClass):
            nonlocal iterator
            nonlocal token
            assert token[1] == '{'

            if clazz.doc is None:
                _skip_special_clause()
                return

            visibility = 'private'
            template_clause = None

            for token in iterator:
                # print(f'_parse_class_body iteration {token}')
                if _parse_comment():
                    continue
                if token[1] == '}':
                    break
                if token[1] in ['public', 'private', 'protected', 'signals']:
                    visibility = token[1]
                    if visibility == 'signals':
                        visibility = 'public'
                    assert next(iterator)[1] == ':'
                    continue
                if token[0] == cpp_tokenizer.Type.GENERIC_OPEN:
                    _skip_special_clause()

                if token[0] == cpp_tokenizer.Type.IDENTIFIER:
                    if token[1] == 'enum':
                        while token[1] != ';':
                            token = next(iterator)
                        continue
                    if token[1] == 'template':
                        template_clause = [token]
                        token = next(iterator)
                        template_clause += _skip_special_clause([ cpp_tokenizer.Type.GENERIC_OPEN, cpp_tokenizer.Type.GENERIC_OPEN2 ], [ cpp_tokenizer.Type.GENERIC_CLOSE, cpp_tokenizer.Type.GENERIC_CLOSE2 ])
                        continue

                    modifiers_before = []
                    while True:
                        type_str = _parse_type()
                        if type_str in ["explicit", "static", "constexpr", "virtual"]:
                            modifiers_before.append(type_str)
                            continue
                        break
                    if token[1] == '(':
                        # ctor
                        clazz.methods.append(CppFunction(name=type_str, doc=_consume_comment(), visibility=visibility, args=_skip_special_clause(), modifiers_before=modifiers_before))
                        continue
                    elif token[0] == cpp_tokenizer.Type.IDENTIFIER:
                        name = token[1]
                        token = next(iterator)
                        if token[1] == ';':
                            clazz.fields.append((type_str, name, _consume_comment()))
                            continue
                        if token[1] == '(':
                            clazz.methods.append(CppFunction(name=name, return_type=type_str, doc=_consume_comment(), visibility=visibility, template_clause=template_clause, args=_skip_special_clause(), modifiers_before=modifiers_before))
                            template_clause = None
                            token = next(iterator)
                    while True:
                        if token[1] == ';':
                            break
                        if token[1] == '{':
                            _skip_special_clause()
                            break
                        # consume const noexcept
                        token = next(iterator)


        if _parse_comment():
            continue

        if token == (cpp_tokenizer.Type.IDENTIFIER, 'enum'):
            token = next(iterator) # enum class???
            continue



        if token == (cpp_tokenizer.Type.IDENTIFIER, 'class'):
            token = next(iterator)
            if token[1].startswith("API_"):
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
            if "aui.toolbox" in str(root):
                continue
            if not file.endswith('.h'):
                continue
            if "3rdparty" in root:
                continue
            full_path = Path(root) / file
            try:
                include_dir = full_path
                while include_dir.name != 'src':
                    if include_dir == include_dir.parent:
                        break
                    include_dir = include_dir.parent

                contents = [i for i in _parse(full_path.read_text())]

                for clazz in contents:
                    class_name = clazz.name
                    classes.add(class_name)
                    with mkdocs_gen_files.open(f'reference/{class_name.lower()}.md', 'w') as fos:
                        print(f'# {class_name}', file=fos)
                        print(f'', file=fos)
                        doxygen = parse_doxygen(clazz.doc)

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

                        methods = [i for i in clazz.methods if i.visibility != 'private' and i.doc is not None]
                        if methods:
                            print('## Public Methods', file=fos)
                            methods_grouped = {}
                            for i in methods:
                                methods_grouped.setdefault(i.name, []).append(i)
                            for name, overloads in sorted(methods_grouped.items(), key=lambda x: x[0] if x[0] != class_name else '!!!ctor'):
                                # hack: present the header as invisible block. The header will still appear in TOC and
                                # can be anchor referenced.
                                print(f'<div style="height: 0px; opacity: 0" markdown>', file=fos)
                                print(f'### {name}', file=fos)
                                print(f'</div>', file=fos)
                                for overload in overloads:
                                    print('---', file=fos)
                                    print(f'```cpp', file=fos)
                                    if overload.template_clause:
                                        print(_format_token_sequence([i[1] for i in overload.template_clause]), file=fos)

                                    if overload.modifiers_before:
                                        print(_format_token_sequence(overload.modifiers_before), end=' ', file=fos)

                                    if overload.return_type: # not a constructor
                                        print(f'{overload.return_type} ', end='', file=fos)
                                    print(f'{class_name}::{overload.name}{_format_token_sequence([ i[1] for i in overload.args])}', end='', file=fos)
                                    print('', file=fos)
                                    print(f'```', file=fos)
                                    print(f'', file=fos)
                                    print(f'{overload.doc}', file=fos)

            except Exception as e:
                log.warning(f'Source file {full_path} could not be parsed:', e)



    with mkdocs_gen_files.open('classes.md', 'w') as f:
        classes_alphabet = { }

        for clazz in classes:
            letter = clazz[0]
            if letter == 'A' and clazz[1].isupper():
                # most classes in AUI start with 'A', so it makes less sense to chunk by 'A'. Instead, we'll use the
                # second letter.
                letter = clazz[1]
            letter = letter.upper()
            classes_alphabet.setdefault(letter, []).append(clazz)
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
    assert clazz.methods == [
        CppFunction(return_type='void', name='hello')
    ]

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
    assert clazz.methods == [CppFunction(return_type='void', name='hello', doc='@brief Hello')]

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
        CppFunction(return_type='void', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
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
        CppFunction(return_type='void', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
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
        CppFunction(return_type='void', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
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
        CppFunction(return_type='_<Test>', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
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
        CppFunction(return_type='const _<Test>&', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
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
        CppFunction(return_type='_<Test>', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
    ]

def test_parse_class_const_noexcept():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    _<Test> hello(_<Test> test) const noexcept {
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
        CppFunction(return_type='_<Test>', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
    ]


def test_parse_class_const_less():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    _<Test> hello(_<Test> test) const noexcept {
        if (test < 0) {
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
        CppFunction(return_type='_<Test>', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
    ]



def test_parse_class_nodiscard():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    [[nodiscard]]
    _<Test> hello(_<Test> test) const noexcept {
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
        CppFunction(return_type='_<Test>', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
    ]

def test_parse_class_namespace():
    clazz = next(_parse("""
/**
 * @brief Test
 */
class Test {
public:
    /**
     * @brief Hello
     */
    [[nodiscard]]
    test::Test hello(test::Test test) const noexcept {
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
        CppFunction(return_type='test::Test', name='hello', doc='@brief Hello'),
        CppFunction(return_type='int', name='world', doc='@brief World'),
    ]

def test_parse_class9():
    assert next(_parse("""
/**
 * @brief test
 */
class API_AUI_CORE Test {}
    """)).name == "Test"

def test_parse_aobject():
    clazz = next(_parse((Path('test_data') / 'AObject.h').read_text()))
    assert clazz.name == "AObject"
    for i in clazz.methods:
        i.doc = None
        # print(i, ',')

    assert [str(i) for i in clazz.methods] == [
        "CppFunction('AObject', None, None)",
        "CppFunction('AObject', '~', None)",
        "CppFunction('disconnect', 'void', None)",
        "CppFunction('connect', 'decltype(auto)', None)",
        "CppFunction('connect', 'decltype(auto)', None)",
        "CppFunction('connect', 'void', None)",
        "CppFunction('biConnect', 'void', None)",
        "CppFunction('connect', 'decltype(auto)', None)",
        "CppFunction('connect', 'decltype(auto)', None)",
        "CppFunction('connect', 'decltype(auto)', None)",
        "CppFunction('connect', 'decltype(auto)', None)",
        "CppFunction('connect', 'void', None)",
        "CppFunction('setSignalsEnabled', 'void', None)",
        "CppFunction('isSignalsEnabled', 'bool', None)",
        "CppFunction('getThread', 'const _<AAbstractThread>&', None)",
        "CppFunction('isSlotsCallsOnlyOnMyThread', 'bool', None)",
        "CppFunction('moveToThread', 'void', None)",
        "CppFunction('setSlotsCallsOnlyOnMyThread', 'void', None)",
        "CppFunction('setThread', 'void', None)",
        "CppFunction('isDisconnected', 'bool&', None)",
    ]
