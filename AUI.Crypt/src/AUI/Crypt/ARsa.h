#pragma once
#include "AUI/Rsa.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/AByteBuffer.h"

/**
 * \brief Хранит открытый и/или закрытый ключ RSA
 */
class API_AUI_CRYPT ARsa
{
private:
	void* mRsa = nullptr;


	explicit ARsa(void* rsa);

public:
	~ARsa();

	_<AByteBuffer> encrypt(_<AByteBuffer> in);
	_<AByteBuffer> decrypt(_<AByteBuffer> in);

	size_t getKeyLength() const;

	_<AByteBuffer> getPrivateKeyPEM() const;
	_<AByteBuffer> getPublicKeyPEM() const;
	
	static _<ARsa> generate(int bits = 0x800);
	static _<ARsa> fromPrivateKeyPEM(_<AByteBuffer> buffer);
	static _<ARsa> fromPublicKeyPEM(_<AByteBuffer> buffer);
};