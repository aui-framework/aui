// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once


#include "INode.h"
#include "ShadingLanguage/Lang/Token/KeywordToken.h"
#include "VariableDeclarationNode.h"
#include "AUI/Common/AMap.h"

class NonIndexedAttributesDeclarationNode: public INode {
public:
    using Fields = AVector<_<VariableDeclarationNode>>;
    NonIndexedAttributesDeclarationNode(KeywordToken type, Fields fields) :
            mType(type),
            mFields(std::move(fields)) {}

    ~NonIndexedAttributesDeclarationNode() override = default;

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
