#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
from enum import Enum, auto
from itertools import count


class Type(Enum):
    UNKNOWN = auto()
    COMMENT = auto()
    PREPROCESSOR = auto()
    SEMICOLON = auto()
    IDENTIFIER = auto()
    GENERIC_OPEN = auto()
    GENERIC_CLOSE = auto()
    GENERIC_OPEN2 = auto()
    GENERIC_CLOSE2 = auto()
    LITERAL = auto()
    STRING = auto()
    COLON = auto()
    COLON2 = auto()
    AMPERSAND1 = auto()
    AMPERSAND2 = auto()



def tokenize(input: str):
    i = 0
    output = []

    def peek():
        nonlocal i
        return input[i]
    def take():
        nonlocal i
        out = peek()
        i += 1
        return out

    def make_token(type: Type):
        return (type, input[beginning:i])


    try:
        while i < len(input):
            beginning = i
            if peek() == '/':
                take()
                if peek() == '/':
                    while i < len(input) and peek() != '\n':
                        take()
                    yield make_token(Type.COMMENT)
                elif peek() == '*':
                    take()
                    while True:
                        if take() == '*':
                            if take() == '/':
                                yield make_token(Type.COMMENT)
                                break
            elif peek() == '#':
                while peek() != '\n':
                    if peek() == '\\':
                        take()
                    take()
                yield make_token(Type.PREPROCESSOR)
            elif str.isalpha(peek()) or peek() == '_':
                while str.isalnum(peek()) or peek() == '_':
                    take()
                yield make_token(Type.IDENTIFIER)
            elif peek() == ';':
                take()
                yield make_token(Type.SEMICOLON)
            elif peek() == ':':
                take()
                if peek() == ':':
                    take()
                    yield make_token(Type.COLON2)
                    continue
                yield make_token(Type.COLON)
            elif peek() == '&':
                take()
                if peek() == '&':
                    take()
                    yield make_token(Type.AMPERSAND2)
                    continue
                yield make_token(Type.AMPERSAND1)
            elif peek() in "({[":
                take()
                yield make_token(Type.GENERIC_OPEN)
            elif peek() in ")}]":
                take()
                yield make_token(Type.GENERIC_CLOSE)
            elif peek() in "<":
                take()
                yield make_token(Type.GENERIC_OPEN2)
            elif peek() in ">":
                take()
                yield make_token(Type.GENERIC_CLOSE2)
            elif str.isdigit(peek()):
                while str.isdigit(peek()):
                    take()
                yield make_token(Type.LITERAL)
            elif peek() == '"':
                take()
                while peek() != '"':
                    if peek() == '\\':
                        take()
                    take()
                take()
                yield make_token(Type.STRING)
            elif peek() in "\n \t":
                take()
            else:
                # line = len([i for i in input[0:i] if i == '\n'])
                # raise RuntimeError(f'unexpected token "{take()}" at {line}')
                take()
                yield make_token(Type.UNKNOWN)
    except IndexError:
        pass



def test_simple_header():
    assert [i for i in tokenize("""
/*
 * license notice
 */
#include <AUI/Core.h>
    """)] == [
        (Type.COMMENT, """/*
 * license notice
 */"""),
        (Type.PREPROCESSOR, "#include <AUI/Core.h>")
    ]


def test_class_declaration():
    assert [i for i in tokenize("""
class Test;
    """)] == [
        (Type.IDENTIFIER, "class"),
        (Type.IDENTIFIER, "Test"),
        (Type.SEMICOLON, ";"),
    ]

def test_class_definition():
    assert [i for i in tokenize("""
class Test {

};
    """)] == [
        (Type.IDENTIFIER, "class"),
        (Type.IDENTIFIER, "Test"),
        (Type.GENERIC_OPEN, "{"),
        (Type.GENERIC_CLOSE, "}"),
        (Type.SEMICOLON, ";"),
    ]

def test_string1():
    assert [i for i in tokenize("""
"hello"
    """)] == [
        (Type.STRING, "\"hello\""),
    ]

def test_string2():
    assert [i for i in tokenize("""
"hel\\"lo"
    """)] == [
        (Type.STRING, "\"hel\\\"lo\""),
    ]

def test_int():
    assert [i for i in tokenize("""
123
    """)] == [
        (Type.LITERAL, "123"),
    ]

