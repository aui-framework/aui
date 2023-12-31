//
// Created by alex2 on 6/12/2021.
//

#include "AST.h"

void AST::visit(INodeVisitor& visitor) {
    for (auto& v : mNodes) {
        v->acceptVisitor(visitor);
    }
}
