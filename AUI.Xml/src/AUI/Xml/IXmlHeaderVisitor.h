#pragma once
#include "AUI/Common/SharedPtr.h"

class IXmlHeaderVisitor
{
public:
	virtual void visitAttribute(const AString& name, const AString& value) = 0;
};
