//
// Created by alex2 on 6/29/2021.
//

#include "TemplateOperatorTypenameNode.h"
#include "INodeVisitor.h"

void TemplateOperatorTypenameNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
