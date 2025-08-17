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
import os
import re
from pathlib import Path

from docs.python.generators import regexes, cpp_tokenizer

CPP_CLASS_DEF = re.compile('class( API_\S+)? ([a-zA-Z0-9_$]+)')
assert CPP_CLASS_DEF.match('class Test').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test;').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test {').group(2) == "Test"
assert CPP_CLASS_DEF.match('class API_AUI_CORE Test: Base {').group(2) == "Test"

CPP_COMMENT_LINE = re.compile('\s*\* ?(.*)')
assert CPP_COMMENT_LINE.match('  * Test').group(1) == "Test"
assert CPP_COMMENT_LINE.match('  *  Test').group(1) == " Test"

log = logging.getLogger('mkdocs')

def parse_comment_lines(iterator):
    output = []
    for line in iterator:
        if "/*" in line:
            continue
        if "*/" in line:
            break
        line = CPP_COMMENT_LINE.match(line).group(1)

        output.append(line)
    return "\n".join(output)


class CppVariable:
    def __init__(self, name, type_str, doc = None, visibility = 'public'):
        self.name = name
        self.type_str = type_str
        self.doc = doc
        self.visibility = visibility

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

class DoxygenEntry:
    def __init__(self, doc = None):
        self.doc = doc
        self.location = None

class CppClass:
    def __init__(self):
        self.name = None
        self.doc = None
        self.methods = []
        self.fields = []
        self.location = None
        self.namespace = []

    def namespaced_name(self):
        return "::".join(self.namespace + [self.name])

    def tuple(self):
        return self.name, self.doc, self.methods, self.fields

    def __eq__(self, other):
        return self.tuple() == other.tuple()
    def __str__(self):
        return f'CppClass{self.tuple()}'

def _parse(input: str, location = None | Path):
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
                            clazz.fields.append(CppVariable(name=name, type_str=type_str, doc=_consume_comment(), visibility=visibility))
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
                    _consume_comment()


        prev_doc = last_doc
        if _parse_comment():
            if prev_doc is not None:
                entry = DoxygenEntry(doc=prev_doc)
                entry.location = location
                yield entry
            continue

        if token == (cpp_tokenizer.Type.IDENTIFIER, 'enum'):
            token = next(iterator) # TODO parse enums
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
            clazz.location = location

            while token[1] not in '{;':
                token = next(iterator)
            if token[1] == '{':
                _parse_class_body(clazz)

            if clazz.doc:
                yield clazz

def _scan():
    contents = []
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
                contents += [i for i in _parse(full_path.read_text(), location=full_path)]
            except Exception as e:
                log.exception(f'Source file "{full_path.absolute()}" could not be parsed')
    return contents

index = _scan()
# mapping = {i.name : i for i in index if hasattr(i, "name")}



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

def test_parse_namespace():
    clazz = next(_parse("""
namespace aui {

/**
 * @brief Test
 */
class Test {};

}
    """))
    assert clazz.name == "Test"
    assert clazz.doc == '@brief Test'
    assert clazz.methods == []
    assert clazz.namespace == ["aui"]
