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

from docs.python.generators import cpp_tokenizer, regexes

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

    def tuple(self):
        return self.name, self.type_str, self.doc

    def __str__(self):
        return f'CppVariable{self.tuple()}'

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

class CppEnum:
    def __init__(self, name, doc = None):
        self.name = name
        self.doc = doc
        self.enum_values = []
        self.namespace = []
        self.generic_kind = 'enum'
        self.location = None

    def namespaced_name(self):
        return "::".join(self.namespace + [self.name])

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
        self.generic_kind = None # 'class' | 'struct'
        self.page_url = None
        self.types = []

    def namespaced_name(self):
        return "::".join(self.namespace + [self.name])

    def tuple(self):
        return self.name, self.doc, self.methods, self.fields

    def __eq__(self, other):
        return self.tuple() == other.tuple()
    def __str__(self):
        return f'CppClass{self.tuple()}'

class CppMacro:
    def __init__(self, name: str, doc: str | None, definition: str, location: str):
        self.name = name
        self.doc = doc
        self.definition = definition
        self.location = location
        self.values = []
        self.generic_kind = '#define'

    def namespaced_name(self):
        return self.name

class _Parser:
    def __init__(self, input: str, location = None | Path):
        self.tokens = cpp_tokenizer.tokenize(input)
        self.iterator = iter(self.tokens)
        self.last_doc = None
        self.location = location
        self.last_token = None

    def _parse_comment(self):
        if self.last_token[0] == cpp_tokenizer.Type.COMMENT:
            if not "/**" in self.last_token[1]:
                return True
            self.last_doc = parse_comment_lines(self.last_token[1].split('\n'))
            return True
        return False


    def _consume_doc(self):
        doc = self.last_doc
        self.last_doc = None
        return doc


    def _skip_special_clause(self, open = [ cpp_tokenizer.Type.GENERIC_OPEN ], close = [ cpp_tokenizer.Type.GENERIC_CLOSE ]):
        assert self.last_token[0] in open
        special_open = 1
        out = [ self.last_token ]
        while special_open > 0:
            self.last_token = next(self.iterator)
            out.append(self.last_token)
            if self.last_token[0] in open:
                special_open += 1
            elif self.last_token[0] in close:
                special_open -= 1
        return out
    def _parse_type(self):
        out = self.last_token[1]
        while self.last_token[1] in ['const', 'volatile']:
            self.last_token = next(self.iterator)
            out += " " + self.last_token[1]

        self.last_token = next(self.iterator)

        # handle namespace types
        while self.last_token[1] == '::':
            out += self.last_token[1]
            self.last_token = next(self.iterator)
            out += self.last_token[1]
            self.last_token = next(self.iterator)

        is_decltype = 'decltype' in out
        is_template_wizardy = self.last_token[0] in [ cpp_tokenizer.Type.GENERIC_OPEN, cpp_tokenizer.Type.GENERIC_OPEN2 ] and not self.last_token[1] == '('

        if is_decltype or is_template_wizardy:
            out += "".join([i[1] for i in self._skip_special_clause([ cpp_tokenizer.Type.GENERIC_OPEN, cpp_tokenizer.Type.GENERIC_OPEN2 ], [ cpp_tokenizer.Type.GENERIC_CLOSE, cpp_tokenizer.Type.GENERIC_CLOSE2 ])])
            self.last_token = next(self.iterator)
        while self.last_token[1] in '&*':
            out += self.last_token[1]
            self.last_token = next(self.iterator)
        return out

    def _parse_enum(self):
        assert self.last_token == (cpp_tokenizer.Type.IDENTIFIER, 'enum')
        self.last_token = next(self.iterator)
        generic_kind = None
        if self.last_token[1] in ['class', 'struct']:
            generic_kind = f"enum {self.last_token[1]}"
            self.last_token = next(self.iterator)
        name = self.last_token[1]
        self.last_token = next(self.iterator)
        if self.last_token[1] == ':':
            while self.last_token[1] != '{':
                self.last_token = next(self.iterator)
        elif self.last_token[1] == ';':
            # just a enum definition, ignore.
            return None
        assert self.last_token[1] == '{'
        out = CppEnum(name=name, doc=self._consume_doc())
        if generic_kind:
            out.generic_kind = generic_kind
        while True:
            if self.last_token[1] == '}':
                break
            self.last_token = next(self.iterator)
            if self.last_token[1] == '}':
                break

            if self._parse_comment():
                continue
            if self.last_token[0] == cpp_tokenizer.Type.PREPROCESSOR:
                continue
            assert self.last_token[0] == cpp_tokenizer.Type.IDENTIFIER
            enumeration_name = self.last_token[1]
            while self.last_token[1] not in ',}':
                self.last_token = next(self.iterator)
            out.enum_values.append((enumeration_name, self._consume_doc()))
        return out





    def _parse_class_body(self, clazz: CppClass):
        assert self.last_token[1] == '{'

        if clazz.doc is None:
            self._skip_special_clause()
            return

        visibility = 'private' if clazz.generic_kind == 'class' else 'public'
        template_clause = None
        template_clause2 = None

        for self.last_token in self.iterator:
            # print(f'_parse_class_body iteration {self.last_token}')
            template_clause = template_clause2
            template_clause2 = None

            if self._parse_comment():
                continue
            if self.last_token[1] == '}':
                break
            if self.last_token[1] in ['public', 'private', 'protected', 'signals']:
                visibility = self.last_token[1]
                if visibility == 'signals':
                    visibility = 'public'
                assert next(self.iterator)[1] == ':'
                continue
            if self.last_token[0] == cpp_tokenizer.Type.GENERIC_OPEN:
                self._skip_special_clause()

            if self.last_token[0] == cpp_tokenizer.Type.IDENTIFIER:
                if self.last_token[1] == 'enum':
                    e = self._parse_enum()
                    if visibility != 'private':
                        clazz.types.append(e)
                    continue
                if self.last_token[1] == 'template':
                    template_clause2 = self._parse_template()
                    continue
                if self.last_token[1] in ['class', 'struct']:
                    e = self._parse_class()
                    e.location = self.location
                    if visibility != 'private':
                        clazz.types.append(e)
                    continue


                modifiers_before = []
                while True:
                    type_str = self._parse_type()
                    if type_str in ["explicit", "static", "inline", "constexpr", "virtual"]:
                        modifiers_before.append(type_str)
                        continue
                    break
                if self.last_token[1] == '(':
                    # ctor
                    clazz.methods.append(CppFunction(name=type_str, doc=self._consume_doc(), visibility=visibility, args=self._skip_special_clause(), modifiers_before=modifiers_before))
                    continue
                elif self.last_token[0] == cpp_tokenizer.Type.IDENTIFIER:
                    name = self.last_token[1]
                    self.last_token = next(self.iterator)
                    if visibility != 'private':
                        if self.last_token[1] in [';', '{', '=']:
                            clazz.fields.append(CppVariable(name=name, type_str=type_str, doc=self._consume_doc(), visibility=visibility))
                            if self.last_token[1] != ';':
                                self.last_token = next(self.iterator)
                            continue
                        if self.last_token[1] == '(':
                            clazz.methods.append(CppFunction(name=name, return_type=type_str, doc=self._consume_doc(), visibility=visibility, template_clause=template_clause, args=self._skip_special_clause(), modifiers_before=modifiers_before))
                            template_clause = None
                            self.last_token = next(self.iterator)
                while True:
                    if self.last_token[1] == ';':
                        break
                    if self.last_token[1] == '{':
                        self._skip_special_clause()
                        break
                    # consume const noexcept
                    self.last_token = next(self.iterator)
                self._consume_doc()

    def _parse_template(self):
        assert self.last_token == (cpp_tokenizer.Type.IDENTIFIER, 'template')
        template_clause2 = [self.last_token]
        while self.last_token[1] != '<':
            self.last_token = next(self.iterator)
        template_clause2 += self._skip_special_clause(
            [cpp_tokenizer.Type.GENERIC_OPEN, cpp_tokenizer.Type.GENERIC_OPEN2],
            [cpp_tokenizer.Type.GENERIC_CLOSE, cpp_tokenizer.Type.GENERIC_CLOSE2])
        return template_clause2

    def _parse_class(self):
        assert self.last_token[0] == cpp_tokenizer.Type.IDENTIFIER and self.last_token[1] in ['class', 'struct']
        kind = self.last_token[1]
        self.last_token = next(self.iterator)
        if self.last_token[1].startswith("API_"):
            # export macro, ignore
            self.last_token = next(self.iterator)
        if self.last_token[0] == cpp_tokenizer.Type.GENERIC_OPEN: # [[deprecated]]
            self._skip_special_clause()
            self.last_token = next(self.iterator)

        assert self.last_token[0] == cpp_tokenizer.Type.IDENTIFIER
        clazz = CppClass()
        clazz.generic_kind = kind
        clazz.name = self.last_token[1]
        clazz.doc = self._consume_doc()
        clazz.location = self.location

        while self.last_token[1] not in '{;':
            self.last_token = next(self.iterator)
        if self.last_token[1] == '{':
            self._parse_class_body(clazz)
        return clazz

    def _parse_file(self):
        for self.last_token in self.iterator:
            # print(f'_parse_file iteration {self.last_token}')
            prev_doc = self.last_doc

            if self.last_token[1] == '}':
                break

            if self.last_token[1] == '{':
                self._skip_special_clause()
                continue

            if self._parse_comment():
                if prev_doc is not None:
                    entry = DoxygenEntry(doc=prev_doc)
                    entry.location = self.location
                    yield entry
                continue

            if self.last_token[0] == cpp_tokenizer.Type.PREPROCESSOR:
                if m := regexes.MACRO_DEFINE.match(self.last_token[1]):
                    doc = self._consume_doc()
                    if doc is not None:
                        yield CppMacro(name=m.group(1), doc=doc, definition=self.last_token[1], location=self.location)
                    continue


            if self.last_token == (cpp_tokenizer.Type.IDENTIFIER, 'enum'):
                e = self._parse_enum()
                e.location = self.location
                if e.doc:
                    yield e
                continue

            if self.last_token == (cpp_tokenizer.Type.IDENTIFIER, 'template'):
                self._parse_template()
                continue

            if self.last_token == (cpp_tokenizer.Type.IDENTIFIER, 'namespace'):
                self.last_token = next(self.iterator)
                assert self.last_token[0] == cpp_tokenizer.Type.IDENTIFIER
                namespace = [self.last_token[1]]
                for self.last_token in self.iterator:
                    match self.last_token[1]:
                        case '{':
                            break
                        case ';':
                            break
                        case '=': # namespace alias
                            break
                        case '::':
                            self.last_token = next(self.iterator)
                            assert self.last_token[0] == cpp_tokenizer.Type.IDENTIFIER
                            namespace.append(self.last_token[1])
                        case _:
                            raise RuntimeError(f'Unexpected token {self.last_token} in namespace declaration')

                if self.last_token[1] == ';': # namespace declaration?
                    continue

                for i in self._parse_file():
                    if hasattr(i, 'name'):
                        i.namespace = namespace + i.namespace
                    yield i

            if self.last_token[0] == cpp_tokenizer.Type.IDENTIFIER and self.last_token[1] in ['class', 'struct']:
                clazz = self._parse_class()
                if clazz.doc:
                    yield clazz

    def parse(self):
        return self._parse_file()

def _parse(input: str, location = None | Path):
    return _Parser(input=input, location=location).parse()

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
    assert clazz.types[0].name == 'Kek'

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
    ]

def test_parse_namespace1():
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

def test_parse_namespace2():
    clazz = [i for i in _parse("""
namespace aui {

namespace impl {
/**
 * @brief Test
 */
class Impl {};
}

/**
 * @brief Test
 */
class Test {};

}


/**
 * @brief Test
 */
class AButton {};
    """)]
    assert clazz[0].name == "Impl"
    assert clazz[0].doc == '@brief Test'
    assert clazz[0].methods == []
    assert clazz[0].namespace == ["aui", "impl"]
    assert clazz[1].name == "Test"
    assert clazz[1].doc == '@brief Test'
    assert clazz[1].methods == []
    assert clazz[1].namespace == ["aui"]
    assert clazz[2].name == "AButton"
    assert clazz[2].doc == '@brief Test'
    assert clazz[2].methods == []
    assert clazz[2].namespace == []

def test_parse_namespace3():
    clazz = [i for i in _parse("""
namespace aui::impl {
/**
 * @brief Test
 */
class Impl {};
}
    """)]
    assert clazz[0].name == "Impl"
    assert clazz[0].doc == '@brief Test'
    assert clazz[0].methods == []
    assert clazz[0].namespace == ["aui", "impl"]

def test_parse_field_initializer():
    clazz = [i for i in _parse("""
namespace aui::impl {
/**
 * @brief Test
 */
class Impl {
public:
    /**
     * @brief Test field 1
     */
    AString field1{};
    
    /**
     * @brief Test field 2
     */
    AString field2{};
    
    /**
     * @brief Test field 3
     */
    AString field3;
    
    /**
     * @brief Test field 4
     */
    AString field4 = "hello";
    
    /**
     * @brief Test field 5
     */
    AString field5{"hello"};
};
}
    """)]
    assert clazz[0].name == "Impl"
    assert clazz[0].doc == '@brief Test'
    assert clazz[0].methods == []
    assert [str(i) for i in clazz[0].fields] == [
        str(CppVariable(name="field1", type_str="AString", doc="@brief Test field 1")),
        str(CppVariable(name="field2", type_str="AString", doc="@brief Test field 2")),
        str(CppVariable(name="field3", type_str="AString", doc="@brief Test field 3")),
        str(CppVariable(name="field4", type_str="AString", doc="@brief Test field 4")),
        str(CppVariable(name="field5", type_str="AString", doc="@brief Test field 5")),
    ]

def test_macro1():
    macro = [i for i in _parse("""
/**
 * @brief Test
 */
#define AUI_MACRO(k, v) k v
    """)]
    assert macro[0].name == "AUI_MACRO"
    assert macro[0].doc == '@brief Test'
    assert macro[0].definition == '#define AUI_MACRO(k, v) k v'

def test_class_types1():
    clazz = [i for i in _parse("""
/**
 * @brief Test
 */
class Impl {
public:
    /**
     * @brief Enum
     */
    enum class Enum {
        /**
         * @brief Value 1
         */
        VALUE_1, 
        
        /**
         * @brief Value 2
         */
        VALUE_2, 
    };
};
    """)]
    assert clazz[0].name == "Impl"
    assert clazz[0].doc == '@brief Test'
    assert clazz[0].types[0].name == "Enum"
    assert clazz[0].types[0].enum_values[0] == ('VALUE_1', '@brief Value 1')
    assert clazz[0].types[0].enum_values[1] == ('VALUE_2', '@brief Value 2')


def test_class_types2():
    clazz = [i for i in _parse("""
/**
 * @brief Test
 */
class Impl {
public:
    /**
     * @brief Nested
     */
    struct Nested {
        /**
         * @brief Test field
         */
        AString field; 
    };
};
    """)]
    assert clazz[0].name == "Impl"
    assert clazz[0].doc == '@brief Test'
    assert clazz[0].types[0].name == "Nested"
    assert [str(i) for i in clazz[0].types[0].fields] == [
        str(CppVariable(name="field", type_str="AString", doc="@brief Test field"))
    ]



def test_class_template():
    clazz = [i for i in _parse("""
/**
 * @brief Test
 */
template<class T, class Allocator>
class Impl {
public:
};
    """)]
    assert clazz[0].name == "Impl"
    assert clazz[0].doc == '@brief Test'
