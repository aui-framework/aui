/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

AByteBuffer ARsa::encrypt(AByteBufferView in)
{
    AByteBuffer buf;
	buf.reserve(in.size() + 0x1000);
	for (auto it = in.begin(); it != in.end();) {
		auto toRead = glm::min(size_t(in.end() - it), getKeyLength() - RSA_PKCS1_PADDING_SIZE);
		int r = RSA_public_encrypt(toRead, reinterpret_cast<const unsigned char*>(it),
			reinterpret_cast<unsigned char*>(buf.data() + buf.getSize()), static_cast<RSA*>(mRsa),
			RSA_PKCS1_PADDING);

		if (r < 0)
			throw AException("Could not RSA_public_encrypt");

		it += toRead;
		
		buf.setSize(buf.getSize() + r);
	}

	return buf;
}

AByteBuffer ARsa::decrypt(AByteBufferView in)
{
    AByteBuffer buf;
	buf.reserve(in.size() + 0x1000);
    for (auto it = in.begin(); it != in.end();) {
		auto toRead = glm::min(size_t(in.end() - it), getKeyLength());
		int r = RSA_private_decrypt(toRead, reinterpret_cast<const unsigned char*>(it),
			reinterpret_cast<unsigned char*>(buf.data() + buf.getSize()), static_cast<RSA*>(mRsa),
			RSA_PKCS1_PADDING);
		if (r < 0)
			throw AException("Could not RSA_private_decrypt");

        it += toRead;

		buf.setSize(buf.getSize() + r);
	}
	return buf;
}

size_t ARsa::getKeyLength() const
{
	return RSA_size(static_cast<RSA*>(mRsa));
}

AByteBuffer ARsa::getPrivateKeyPEM() const
{
    AByteBuffer byteBuffer;
    BIO* bioKey = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(bioKey, (RSA*)mRsa, EVP_des_ede3_cbc(), nullptr, 0, nullptr, gKey);
	char* data;
	long size = BIO_get_mem_data(bioKey, &data);

	byteBuffer.write(data, size);
	BIO_free(bioKey);

	return byteBuffer;
}
AByteBuffer ARsa::getPublicKeyPEM() const
{
    AByteBuffer byteBuffer;
	BIO* bioKey = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPublicKey(bioKey, (RSA*)mRsa);

	char* data;
	long size = BIO_get_mem_data(bioKey, &data);

	byteBuffer.write(data, size);
	BIO_free(bioKey);

	return byteBuffer;
}

_<ARsa> ARsa::generate(int bits)
{
	return aui::ptr::manage(new ARsa(RSA_generate_key(bits, RSA_F4, nullptr, nullptr)));
}

_<ARsa> ARsa::fromPrivateKeyPEM(AByteBufferView buffer)
{
	BIO* inputBuffer = BIO_new_mem_buf(buffer.data(), buffer.size());
	RSA* rsa = nullptr;
	PEM_read_bio_RSAPrivateKey(inputBuffer, (RSA**)&rsa, nullptr, gKey);
	BIO_free(inputBuffer);

	if (rsa == nullptr)
		throw AException("Could not create RSA private key");

	return aui::ptr::manage(new ARsa(rsa));
}

_<ARsa> ARsa::fromPublicKeyPEM(AByteBufferView buffer)
{
	BIO* inputBuffer = BIO_new_mem_buf(buffer.data(), buffer.size());
	RSA* rsa = nullptr;
	PEM_read_bio_RSAPublicKey(inputBuffer, (RSA**)&rsa, nullptr, nullptr);
	BIO_free(inputBuffer);

	if (rsa == nullptr)
		throw AException("Could not create RSA public key");

	return aui::ptr::manage(new ARsa(rsa));
}
