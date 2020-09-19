#pragma once
#include "AUI/Crypt.h"
#include "AUI/Common/AByteBuffer.h"
#include "AX509.h"

class API_AUI_CRYPT AX509Store
{
private:
	void* mX509Store;
	ADeque<_<AX509>> mCertificates;
	
public:
	AX509Store();
	~AX509Store();

	bool validate(_<AX509> cert);
	void addCert(_<AX509> cert);
};
