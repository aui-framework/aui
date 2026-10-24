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

/**
 * @brief Xml header visitor.
 * @ingroup xml
 * @details
 * Receives attributes of the `<?xml version="1.0" encoding="UTF-8"?>` header. Return it from
 * `IXmlDocumentVisitor::visitHeader`. See [xml-parsing].
 */
class IXmlHeaderVisitor
{
public:
	virtual void visitAttribute(const AString& name, const AString& value) = 0;
};
