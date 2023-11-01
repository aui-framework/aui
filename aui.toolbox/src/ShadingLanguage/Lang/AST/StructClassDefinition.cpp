//
// Created by alex2 on 6/17/2021.
//

#include "StructClassDefinition.h"
#include "INodeVisitor.h"

void StructClassDefinition::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
