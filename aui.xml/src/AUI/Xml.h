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

#pragma once
#pragma warning(disable: 4251)

#include "AUI/api.h"

/**
 * @defgroup xml aui::xml
 * @brief XML parser library.
 * @details
 * aui::xml is a small event-driven (SAX-like) XML reader. It does not build a document tree; instead, `AXml::read`
 * walks the input once and reports everything it meets to your visitors:
 *
 * - [IXmlDocumentVisitor] receives the top-level elements and, optionally, the `<?xml ... ?>` header;
 * - [IXmlEntityVisitor] receives attributes, text and nested elements of one element;
 * - [IXmlHeaderVisitor] receives the attributes of the `<?xml ... ?>` header.
 *
 * ```cpp
 * #include <AUI/Xml/AXml.h>
 * #include <AUI/IO/AStringStream.h>
 *
 * struct Item: IXmlEntityVisitor {
 *     void visitAttribute(const AString& name, AString value) override {
 *         ALogger::info("Xml") << name << " = " << value;
 *     }
 * };
 *
 * struct Document: IXmlDocumentVisitor {
 *     _<IXmlEntityVisitor> visitEntity(AString entityName) override {
 *         return _new<Item>();
 *     }
 * };
 *
 * AXml::read(_new<AStringStream>(AString("<item id=\"1\" title=\"Hello\"/>")), _new<Document>());
 * ```
 *
 * To link aui::xml to your project, add the `xml` component to `auib_import` and link `aui::xml`:
 *
 * ```cmake
 * auib_import(aui https://github.com/aui-framework/aui COMPONENTS core xml VERSION ${AUI_VERSION})
 * aui_link(${PROJECT_NAME} PRIVATE aui::xml)
 * ```
 *
 * See [xml-parsing] for a walkthrough and the limitations of the parser. There is no XML writer in aui::xml.
 */
