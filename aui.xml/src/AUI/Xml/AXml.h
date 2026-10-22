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
#include "IXmlDocumentVisitor.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/Xml.h"


/**
 * @brief Common aui::xml namespace.
 * @ingroup xml
 * @details
 * Contains the single entry point of the library, `AXml::read`. See [xml-parsing].
 */
namespace AXml
{

    /**
     * @brief Parses xml from the input stream to the IXmlDocumentVisitor.
     * @ingroup xml
     * @param is input stream to read from. It is read to the end.
     * @param visitor receives the contents of the document.
     * @details
     * Throws [AXmlParseError] if an attribute value is not enclosed in quotes. Anything else the parser does not
     * recognize is passed to the visitor as is or silently skipped; see [xml-parsing] for details.
     */
	void API_AUI_XML read(const _<IInputStream>& is, const _<IXmlDocumentVisitor>& visitor);
}
