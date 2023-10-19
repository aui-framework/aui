//
// Created by alex2 on 6/29/2021.
//

#pragma once

#include <ShadingLanguage/Lang/AST/ExpressionNode.h>
#include <AUI/Common/AString.h>

class TemplateOperatorTypenameNode: public ExpressionNode {
private:
    AString mTemplateArg;
    AString mClassName;

public:
    TemplateOperatorTypenameNode(const AString& templateArg, const AString& className) : mTemplateArg(templateArg),
                                                                                         mClassName(className) {}

    const AString& getTemplateArg() const {
        return mTemplateArg;
    }

    const AString& getClassName() const {
        return mClassName;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


