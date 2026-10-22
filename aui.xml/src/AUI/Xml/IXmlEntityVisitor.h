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
#include "AUI/Common/SharedPtr.h"
#include "AUI/Common/AString.h"



/**
 * @brief Visits an xml entity (element).
 * @ingroup xml
 * @details
 * One instance corresponds to one element. The parser calls its methods in document order:
 *
 * - `visitAttribute` for each attribute in the opening tag. An attribute without a value gets an empty string.
 * - `visitTextEntity` for text between tags.
 * - `visitEntity` for each nested element. Return a visitor which will receive the nested element, or nullptr to
 *   skip the element along with its contents.
 *
 * All methods have empty default implementations, so override only what you need.
 *
 * ```cpp
 * struct Point: IXmlEntityVisitor {
 *     int x = 0, y = 0;
 *     void visitAttribute(const AString& name, AString value) override {
 *         if (name == "x") x = value.toInt().valueOr(0);
 *         if (name == "y") y = value.toInt().valueOr(0);
 *     }
 * };
 * ```
 *
 * See [xml-parsing].
 */
class IXmlEntityVisitor
{
public:
	virtual void visitAttribute(const AString& name, AString value) {};
	virtual _<IXmlEntityVisitor> visitEntity(AString entityName) { return nullptr; };
	virtual void visitTextEntity(const AString& entity) {};
    virtual ~IXmlEntityVisitor() = default;
};
