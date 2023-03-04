//
// Created by alex2 on 6/12/2021.
//

#pragma once

class INodeVisitor;

class INode {
private:
    unsigned mLineNumber = 0;

public:
    INode() = default;
    virtual ~INode() = default;

    virtual void acceptVisitor(INodeVisitor& v) = 0;

    void setLineNumber(unsigned int lineNumber) {
        mLineNumber = lineNumber;
    }

    [[nodiscard]]
    unsigned getLineNumber() const {
        return mLineNumber;
    }
};
