#pragma once
#include "IXmlDocumentVisitor.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/Xml.h"


namespace AXml
{
	void API_AUI_XML read(_<IInputStream> is, _<IXmlDocumentVisitor> visitor);
}
