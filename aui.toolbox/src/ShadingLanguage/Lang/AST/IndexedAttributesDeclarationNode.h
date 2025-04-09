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


#include "INode.h"
#include "ShadingLanguage/Lang/Token/KeywordToken.h"
#include "VariableDeclarationNode.h"
#include "AUI/Common/AMap.h"

class IndexedAttributesDeclarationNode: public INode {
public:
    using Fields = AMap<int, _<VariableDeclarationNode>>;
    IndexedAttributesDeclarationNode(KeywordToken type, Fields fields) :
        mType(type),
        mFields(std::move(fields)) {}

    ~IndexedAttributesDeclarationNode() override = default;

    void acceptVisitor(INodeVisitor& v) override;

    KeywordToken::Type type() const {
        return mType.getType();
    }

    const Fields& fields() const {
        return mFields;
    }

private:
    KeywordToken mType;
    Fields mFields;
};
