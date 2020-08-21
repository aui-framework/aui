#pragma once
#include <cstdio>
#include "AUI/Core.h"
#include "IInputStream.h"

class AString;

class API_AUI_CORE FileInputStream: public IInputStream
{
private:
	FILE* mFile;
	
public:
	FileInputStream(const AString& path);
	virtual ~FileInputStream();

	int read(char* dst, int size) override;
};
