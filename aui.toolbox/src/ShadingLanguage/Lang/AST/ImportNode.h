//
// Created by alex2 on 6/24/2021.
//

#pragma once

#include "ExpressionNode.h"

class AST;

class ImportNode: public ExpressionNode {
private:
    AArc<AST> mImportedAST;

public:
    explicit ImportNode(const AArc<AST>& importedAst) : mImportedAST(importedAst) {

    }

    const AArc<AST>& importedAst() const {
        return mImportedAST;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


