//
// Created by alex2 on 6/24/2021.
//

#pragma once

#include "ExpressionNode.h"

class AST;

class ImportNode: public ExpressionNode {
private:
    _<AST> mImportedAST;

public:
    explicit ImportNode(const _<AST>& importedAst) : mImportedAST(importedAst) {

    }

    const _<AST>& importedAst() const {
        return mImportedAST;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


