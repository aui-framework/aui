#pragma once
#include "AAbstractSocket.h"
#include "ATcpSocket.h"

class API_AUI_NETWORK ATcpServerSocket: public AAbstractSocket
{
protected:
	int createSocket() override;
public:
	ATcpServerSocket(uint16_t serverPort);
	~ATcpServerSocket() override;

	_<ATcpSocket> accept();
};
