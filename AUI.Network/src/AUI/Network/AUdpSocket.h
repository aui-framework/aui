#pragma once

#include <cstdint>
#include <AUI/Common/AString.h>
#include <AUI/Network.h>


#include "AAbstractSocket.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/IO/IOException.h"
#include "AUI/IO/IOutputStream.h"

#include "AInet4Address.h"

class AByteBuffer;

class API_AUI_NETWORK AUdpSocket : public AAbstractSocket
{
public:

    /**
     * \brief Creates socket
     * \param port port
     */
    AUdpSocket(uint16_t port);


    /**
     * \brief Creates socket with random port (used for "client" socket)
     */
	AUdpSocket();

	AUdpSocket(const AUdpSocket&) = delete;
    ~AUdpSocket() override = default;


    /**
     * \brief Sends data by address.
     * \param buf data buffer
     * \param dst destination delivery address
     */
	void write(const AByteBuffer& buf, const AInet4Address& dst);

	/**
	 * \brief Read data.
	 * \param buf (out) received data
	 * \param dst (out) sender address
	 */
	void read(AByteBuffer& buf, AInet4Address& dst);

protected:
	int createSocket() override;
	
private:
	AInet4Address mSelf;

};
