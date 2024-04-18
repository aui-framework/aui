//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <ShadingLanguage/Lang/AST/AST.h>
#include <ShadingLanguage/Lang/AST/ExpressionNode.h>
#include <AUI/IO/IInputStream.h>
#include "AUI/IO/APath.h"

namespace aui::sl {
    _<AST> parseCode(_<IInputStream> is, APath fileDir = APath::workingDir());
    _<ExpressionNode> parseExpression(const AString& text);
    AVector<_<INode>> parseCodeBlock(const AString& text);
};


