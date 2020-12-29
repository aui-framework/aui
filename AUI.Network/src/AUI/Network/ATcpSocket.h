#pragma once

#include <AUI/Network.h>


#include "AAbstractSocket.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/IO/IOutputStream.h"

#include "AInet4Address.h"

class AByteBuffer;

class API_AUI_NETWORK ATcpSocket: public AAbstractSocket, public IInputStream, public IOutputStream
{
friend class ATcpServerSocket;
public:

	/**
	 * \brief ������ TCP ������-�������
	 * \param destinationAddress ���� ������������
	 */
	ATcpSocket(const AInet4Address& destinationAddress);

	~ATcpSocket() override;

	int read(char* dst, int size) override;
	int write(const char* buffer, int size) override;


protected:
	ATcpSocket(int handle, const AInet4Address& selfAddr)
		: AAbstractSocket(handle, selfAddr)
	{
	}
	
	int createSocket() override;
};
