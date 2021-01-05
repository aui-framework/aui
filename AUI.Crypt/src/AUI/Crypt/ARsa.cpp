/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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

_<AByteBuffer> ARsa::encrypt(_<AByteBuffer> in)
{
	auto buf = _new<AByteBuffer>();
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

_<AByteBuffer> ARsa::decrypt(_<AByteBuffer> in)
{
	auto buf = _new<AByteBuffer>();
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

_<AByteBuffer> ARsa::getPrivateKeyPEM() const
{
	BIO* bioKey = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPrivateKey(bioKey, (RSA*)mRsa, EVP_des_ede3_cbc(), nullptr, 0, nullptr, gKey);
	auto byteBuffer = _new<AByteBuffer>();
	char* data;
	long size = BIO_get_mem_data(bioKey, &data);

	byteBuffer->put(data, size);
	BIO_free(bioKey);

	return byteBuffer;
}
_<AByteBuffer> ARsa::getPublicKeyPEM() const
{
	BIO* bioKey = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPublicKey(bioKey, (RSA*)mRsa);
	auto byteBuffer = _new<AByteBuffer>();
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

_<ARsa> ARsa::fromPrivateKeyPEM(_<AByteBuffer> buffer)
{
	BIO* inputBuffer = BIO_new_mem_buf(buffer->getCurrentPosAddress(), buffer->getAvailable());
	RSA* rsa = nullptr;
	PEM_read_bio_RSAPrivateKey(inputBuffer, (RSA**)&rsa, nullptr, gKey);
	BIO_free(inputBuffer);

	if (rsa == nullptr)
		throw AException("Could not create RSA private key");

	return _<ARsa>(new ARsa(rsa));
}

_<ARsa> ARsa::fromPublicKeyPEM(_<AByteBuffer> buffer)
{
	BIO* inputBuffer = BIO_new_mem_buf(buffer->getCurrentPosAddress(), buffer->getAvailable());
	RSA* rsa = nullptr;
	PEM_read_bio_RSAPublicKey(inputBuffer, (RSA**)&rsa, nullptr, nullptr);
	BIO_free(inputBuffer);

	if (rsa == nullptr)
		throw AException("Could not create RSA public key");

	return _<ARsa>(new ARsa(rsa));
}
