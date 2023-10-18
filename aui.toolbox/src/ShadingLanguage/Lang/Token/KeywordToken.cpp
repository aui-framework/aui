//
// Created by alex2 on 6/12/2021.
//

#include "KeywordToken.h"
#include <AUI/Common/AMap.h>

KeywordToken::Type KeywordToken::fromName(const AString& name) {
    const static AMap<AString, KeywordToken::Type> map = {
            { "none",             NONE },

            { "auto",             AUTO },
            { "break",            BREAK },
            { "case",             CASE },
            { "const",            CONST },
            { "continue",         CONTINUE },
            { "default",          DEFAULT },
            { "do",               DO },
            { "else",             ELSE },
            { "enum",             ENUM },
            { "extern",           EXTERN },
            { "for",              FOR },
            { "goto",             GOTO },
            { "if",               IF },
            { "import",           IMPORT },
            { "return",           RETURN },
            { "signed",           SIGNED },
            { "sizeof",           SIZEOF },
            { "static",           STATIC },
            { "struct",           STRUCT },
            { "switch",           SWITCH },
            { "typedef",          TYPEDEF },
            { "union",            UNION },
            { "unsigned",         UNSIGNED },
            { "volatile",         VOLATILE },
            { "while",            WHILE },
            { "asm",              ASM },
            { "const_cast",       CONST_CAST },
            { "export",           EXPORT },
            { "mutable",          MUTABLE },
            { "private",          PRIVATE },
            { "static_cast",      STATIC_CAST },
            { "true",             TRUE },
            { "using",            USING },
            { "delete",           DELETE },
            { "false",            FALSE },
            { "namespace",        NAMESPACE },
            { "protected",        PROTECTED },
            { "template",         TEMPLATE },
            { "try",              TRY },
            { "virtual",          VIRTUAL },
            { "catch",            CATCH },
            { "dynamic_cast",     DYNAMIC_CAST },
            { "friend",           FRIEND },
            { "new",              NEW },
            { "public",           PUBLIC },
            { "this",             THIS },
            { "typeid",           TYPEID },
            { "class",            CLASS },
            { "explicit",         EXPLICIT },
            { "inline",           INLINE },
            { "operator",         OPERATOR },
            { "reinterpret_cast", REINTERPRET_CAST },
            { "throw",            THROW },
            { "typename",         TYPENAME },
            { "nullptr",          NULLPTR },

            { "input",            INPUT },
            { "output",           OUTPUT },
            { "inter",            INTER },
            { "uniform",          UNIFORM },
            { "entry",            ENTRY },
            { "texture",          TEXTURE },
    };

    if (auto c = map.contains(name)) {
        return c->second;
    }
    return NONE;
}

const char* KeywordToken::getName() {
    return "keyword";
}
