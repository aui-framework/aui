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
	 * \brief Инициализировать сокет
	 */
	void init();
	
	/**
	 * \brief Запуск сокета на определённом порте. Используется для
	 *		  ATcpServerSocket и AUdpSocket
	 * \param bindingPort порт
	 */
	void bind(uint16_t bindingPort);


	/**
	 * \brief Создать хэндл сокета. Используйте фукнцию ::socket()
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
