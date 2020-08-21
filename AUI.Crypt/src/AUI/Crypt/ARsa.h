#pragma once
#include "AUI/Crypt.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/ByteBuffer.h"

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

	_<ByteBuffer> encrypt(_<ByteBuffer> in);
	_<ByteBuffer> decrypt(_<ByteBuffer> in);

	size_t getKeyLength() const;

	_<ByteBuffer> getPrivateKeyPEM() const;
	_<ByteBuffer> getPublicKeyPEM() const;
	
	static _<ARsa> generate(int bits = 0x800);
	static _<ARsa> fromPrivateKeyPEM(_<ByteBuffer> buffer);
	static _<ARsa> fromPublicKeyPEM(_<ByteBuffer> buffer);
};