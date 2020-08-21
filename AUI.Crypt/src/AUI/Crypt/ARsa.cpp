#include "ARsa.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "AUI/Common/AException.h"
#include <glm/glm.hpp>


unsigned char gKey[] = { 52, 12, 96, 112, 84, 4, 95, 125, 213, 234, 87, 195, 175, 13, 42, 0 };

ARsa::ARsa(void* rsa): mRsa(rsa)
{
	if (!rsa)
		throw AException("RSA cannot be nullptr");
}

ARsa::~ARsa()
{
	RSA_free(static_cast<RSA*>(mRsa));
}

_<ByteBuffer> ARsa::encrypt(_<ByteBuffer> in)
{
	auto buf = _new<ByteBuffer>();
	buf->reserve(in->getAvailable() + 0x1000);
	while (in->getAvailable()) {
		auto toRead = glm::min(in->getAvailable(), getKeyLength() - RSA_PKCS1_PADDING_SIZE);
		int r = RSA_public_encrypt(toRead, reinterpret_cast<const unsigned char*>(in->getCurrentPosAddress()),
			reinterpret_cast<unsigned char*>(buf->data() + buf->getSize()), static_cast<RSA*>(mRsa),
			RSA_PKCS1_PADDING);

		if (r < 0)
			throw AException("Could not RSA_public_encrypt");

		in->setCurrentPos(in->getCurrentPos() + toRead);
		
		buf->setSize(buf->getSize() + r);
	}
	return buf;
}

_<ByteBuffer> ARsa::decrypt(_<ByteBuffer> in)
{
	auto buf = _new<ByteBuffer>();
	buf->reserve(in->getAvailable() + 0x1000);
	while (in->getAvailable()) {
		auto toRead = glm::min(in->getAvailable(), getKeyLength());
		int r = RSA_private_decrypt(toRead, reinterpret_cast<const unsigned char*>(in->getCurrentPosAddress()),
			reinterpret_cast<unsigned char*>(buf->data() + buf->getSize()), static_cast<RSA*>(mRsa),
			RSA_PKCS1_PADDING);
		if (r < 0)
			throw AException("Could not RSA_private_decrypt");

		in->setCurrentPos(in->getCurrentPos() + toRead);
		buf->setSize(buf->getSize() + r);
	}
	return buf;
}

size_t ARsa::getKeyLength() const
{
	return RSA_size(static_cast<RSA*>(mRsa));
}

_<ByteBuffer> ARsa::getPrivateKeyPEM() const
{
	BIO* bioKey = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPrivateKey(bioKey, (RSA*)mRsa, EVP_des_ede3_cbc(), nullptr, 0, nullptr, gKey);
	auto byteBuffer = _new<ByteBuffer>();
	char* data;
	long size = BIO_get_mem_data(bioKey, &data);

	byteBuffer->put(data, size);
	BIO_free(bioKey);

	return byteBuffer;
}
_<ByteBuffer> ARsa::getPublicKeyPEM() const
{
	BIO* bioKey = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPublicKey(bioKey, (RSA*)mRsa);
	auto byteBuffer = _new<ByteBuffer>();
	char* data;
	long size = BIO_get_mem_data(bioKey, &data);

	byteBuffer->put(data, size);
	BIO_free(bioKey);

	return byteBuffer;
}

_<ARsa> ARsa::generate(int bits)
{
	return _<ARsa>(new ARsa(RSA_generate_key(bits, RSA_F4, nullptr, nullptr)));
}

_<ARsa> ARsa::fromPrivateKeyPEM(_<ByteBuffer> buffer)
{
	BIO* inputBuffer = BIO_new_mem_buf(buffer->getCurrentPosAddress(), buffer->getAvailable());
	RSA* rsa = nullptr;
	PEM_read_bio_RSAPrivateKey(inputBuffer, (RSA**)&rsa, nullptr, gKey);
	BIO_free(inputBuffer);

	if (rsa == nullptr)
		throw AException("Could not create RSA private key");

	return _<ARsa>(new ARsa(rsa));
}

_<ARsa> ARsa::fromPublicKeyPEM(_<ByteBuffer> buffer)
{
	BIO* inputBuffer = BIO_new_mem_buf(buffer->getCurrentPosAddress(), buffer->getAvailable());
	RSA* rsa = nullptr;
	PEM_read_bio_RSAPublicKey(inputBuffer, (RSA**)&rsa, nullptr, nullptr);
	BIO_free(inputBuffer);

	if (rsa == nullptr)
		throw AException("Could not create RSA public key");

	return _<ARsa>(new ARsa(rsa));
}
