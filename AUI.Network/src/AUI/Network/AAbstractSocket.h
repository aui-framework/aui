#pragma once

#include <AUI/Network.h>



#include "AInet4Address.h"
#include "AUI/Common/AString.h"

class API_AUI_NETWORK AAbstractSocket
{	
private:
	int mHandle = 0;
	AInet4Address mSelfAddress;

	
protected:
	static AString getErrorString();

	[[nodiscard]] inline int getHandle() const
	{
		return mHandle;
	}

	static void handleError(const AString& message, int code);


	AAbstractSocket(int handle, const AInet4Address& selfAddress)
		: mHandle(handle),
		mSelfAddress(selfAddress)
	{
	}
	
	/**
	 * \brief Initialise socket
	 */
	void init();
	
	/**
	 * \brief Bind socket for port. Used for ATcpServerSocket and AUdpSocket
	 * \param bindingPort port
	 */
	void bind(uint16_t bindingPort);


	/**
	 * \brief Create socket handle. Use ::socket()
	 */
	virtual int createSocket() = 0;
	
public:
	AAbstractSocket();
	AAbstractSocket(const AAbstractSocket&) = delete;

	virtual ~AAbstractSocket();

	void close();
	void setTimeout(int secs);

	const AInet4Address& getAddress() const {
		return mSelfAddress;
	}
};
