//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <ShadingLanguage/Lang/AST/AST.h>
#include <ShadingLanguage/Lang/AST/ExpressionNode.h>
#include <AUI/IO/IInputStream.h>
#include "AUI/IO/APath.h"

namespace aui::sl {
    AArc<AST> parseCode(AArc<IInputStream> is, APath fileDir = APath::workingDir());
    AArc<ExpressionNode> parseExpression(const AString& text);
    AVector<AArc<INode>> parseCodeBlock(const AString& text);
};


