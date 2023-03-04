//
// Created by alex2 on 6/13/2021.
//

#include "TemplateOperatorCallNode.h"
#include "INodeVisitor.h"

void TemplateOperatorCallNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
