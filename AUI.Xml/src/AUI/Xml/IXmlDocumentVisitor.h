#pragma once
#include "IXmlEntityVisitor.h"
#include "IXmlHeaderVisitor.h"
#include "AUI/Common/SharedPtr.h"

class IXmlDocumentVisitor
{
public:
	virtual _<IXmlHeaderVisitor> visitHeader() = 0;
	virtual _<IXmlEntityVisitor> visitEntity(const AString& entityName) = 0;
};
