#pragma once

#include <cstdint>
#include <AUI/Common/AString.h>
#include <AUI/Network.h>


#include "AAbstractSocket.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/IO/IOException.h"
#include "AUI/IO/IOutputStream.h"

#include "AInet4Address.h"

class ByteBuffer;

class API_AUI_NETWORK AUdpSocket : public AAbstractSocket
{
public:

	AUdpSocket(const AUdpSocket&) = delete;
	AUdpSocket(uint16_t port);
	AUdpSocket();
	~AUdpSocket() override = default;

	void write(const ByteBuffer& buf, const AInet4Address& dst);
	void read(ByteBuffer& buf, AInet4Address& dst);

protected:
	int createSocket() override;
	
private:
	AInet4Address mSelf;

};
