/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AXml.h"

#include <functional>

#include "AXmlParseError.h"
#include "AUI/Util/ATokenizer.h"

void AXml::read(const _<IInputStream>& is, const _<IXmlDocumentVisitor>& visitor) {
    ATokenizer p(is);

    char c;

    auto throwUnexceptedCharacter = [&]() {
        throw AXmlParseError(
            std::string("Unexpected character '") + p.getLastCharacter() + "' at " + std::to_string(p.getRow()) + ":" +
            std::to_string(p.getColumn()));
    };
    try {
        for (; (c = p.readChar());) {
            switch (c) {
                case '\r':
                case '\n':
                case '\t':
                case ' ':
                    break;
                case '<':
                    // start of a tag
                    switch (p.readChar()) {
                        case '?': {
                            // header
                            auto headerVisitor = visitor->visitHeader();

                            p.readString();   // skip the "xml" word

                            for (; (c = p.readChar()) != '>';) {
                                switch (c) {
                                    case '?':
                                        break;

                                    case ' ':
                                        break;

                                    default: {
                                        p.reverseByte();
                                        auto attrName = p.readString();
                                        bool attrWithNoValue = false;
                                        if (!attrName.empty()) {
                                            for (; (c = p.readChar()) != '=';) {
                                                if (!isblank(c)) {
                                                    if (headerVisitor)
                                                        headerVisitor->visitAttribute(attrName, {});
                                                    attrWithNoValue = true;
                                                    break;
                                                }
                                            }
                                        }
                                        if (!attrWithNoValue) {
                                            AString value;
                                            [&]() {
                                                for (; (c = p.readChar());) {
                                                    switch (c) {
                                                        case '\'':
                                                            value = p.readStringUntilUnescaped('\'');
                                                            return;

                                                        case '"':
                                                            value = p.readStringUntilUnescaped('"');
                                                            return;
                                                        case ' ':
                                                            break;

                                                        default:
                                                            throwUnexceptedCharacter();
                                                    }
                                                }
                                            }();

                                            if (headerVisitor)
                                                headerVisitor->visitAttribute(attrName, value);
                                        }
                                    }
                                }
                            }
                        } break;
                        default:
                            // regular element
                            {
                                p.reverseByte();
                                auto entityName = p.readString();

                                std::function<void(_<IXmlEntityVisitor>)> handleEntity;
                                handleEntity = [&](_<IXmlEntityVisitor> entityVisitor) {
                                    if (!entityVisitor) {
                                        // the visitor is not interested in this element, but it still has to be read
                                        // through to its end; otherwise its contents would leak into the parent.
                                        entityVisitor = _new<IXmlEntityVisitor>();
                                    }
                                    bool endFlag = false;
                                    for (; (c = p.readChar()) != '>';) {
                                        switch (c) {
                                            case '/':
                                                endFlag = true;
                                                break;

                                            case '\r':
                                            case '\n':
                                            case ' ':
                                                break;

                                            default: {
                                                p.reverseByte();
                                                auto attrName = p.readString(ASet<char>{':'});
                                                if (attrName.empty()) {
                                                    p.readChar();
                                                    continue;
                                                }
                                                bool attrWithNoValue = false;
                                                for (; (c = p.readChar()) != '=';) {
                                                    if (!isblank(c)) {
                                                        if (entityVisitor)
                                                            entityVisitor->visitAttribute(attrName, {});
                                                        attrWithNoValue = true;
                                                        break;
                                                    }
                                                }
                                                if (!attrWithNoValue) {
                                                    AString value;
                                                    [&]() {
                                                        for (;;) {
                                                            switch (c = p.readChar()) {
                                                                case '\'':
                                                                    value = p.readStringUntilUnescaped('\'');
                                                                    return;

                                                                case '"':
                                                                    value = p.readStringUntilUnescaped('"');
                                                                    return;
                                                                case ' ':
                                                                    break;

                                                                default:
                                                                    throwUnexceptedCharacter();
                                                            }
                                                        }
                                                    }();

                                                    if (entityVisitor)
                                                        entityVisitor->visitAttribute(attrName, std::move(value));
                                                }
                                            }
                                        }
                                    }
                                    if (!endFlag) {
                                        [&]() {
                                            for (;;) {
                                                c = p.readChar();
                                                switch (c) {
                                                    case ' ':
                                                        break;
                                                    case '\t':
                                                        break;
                                                    case '<':
                                                        c = p.readChar();
                                                        if (c == '/') {
                                                            // end of the element
                                                            p.readString();

                                                            if (p.readChar() != '>')
                                                                throwUnexceptedCharacter();

                                                            return;
                                                        }
                                                        // start of a child element
                                                        {
                                                            p.reverseByte();
                                                            auto entityName = p.readString();
                                                            handleEntity(
                                                                entityVisitor->visitEntity(std::move(entityName)));
                                                        }
                                                        // text following the child element
                                                        {
                                                            auto str = p.readStringUntilUnescaped('<');
                                                            p.reverseByte();
                                                            entityVisitor->visitTextEntity(str);
                                                        }
                                                        break;

                                                    default: {
                                                        // c is the first character of the text; give it back
                                                        p.reverseByte();
                                                        auto str = p.readStringUntilUnescaped('<');
                                                        p.reverseByte();
                                                        entityVisitor->visitTextEntity(str);
                                                    }
                                                }
                                            }
                                        }();
                                    }
                                };
                                handleEntity(visitor->visitEntity(std::move(entityName)));
                            }
                    }

                    break;

                default:
                    p.reverseByte();
                    visitor->visitTextEntity(p.readStringUntilUnescaped('<'));
                    if (p.isEof())
                        return;
                    p.reverseByte();
            }
        }
    } catch (const AEOFException&) {
    }
}
