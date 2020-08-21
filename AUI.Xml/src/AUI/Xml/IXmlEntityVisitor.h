#pragma once
#include "AUI/Common/SharedPtr.h"

class AString;

class IXmlEntityVisitor
{
public:
	virtual void visitAttribute(const AString& name, const AString& value) = 0;
	virtual _<IXmlEntityVisitor> visitEntity(const AString& entityName) = 0;
	virtual void visitTextEntity(const AString& entity) = 0;
};
