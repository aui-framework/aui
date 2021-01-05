/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
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

#include "AX509.h"
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include "AUI/Common/AException.h"

AX509::AX509(void* x509): mX509(x509)
{
}

AX509::~AX509()
{
	X509_free((X509*)mX509);
}

bool AX509::isCA() const
{
	return X509_check_ca((X509*)mX509);
}

bool AX509::checkHost(const AString& name) const
{
	auto x = name.toStdString();
	return X509_check_host((X509*)mX509, x.data(), x.length(), 0, nullptr);
}

bool AX509::checkTrust() const
{
	return X509_trusted((X509*)mX509);
}

_<AX509> AX509::fromPEM(_<AByteBuffer> buffer)
{
	BIO* inputBuffer = BIO_new_mem_buf(buffer->getCurrentPosAddress(), buffer->getAvailable());
	X509* rsa = nullptr;
	PEM_read_bio_X509(inputBuffer, (X509**)&rsa, nullptr, nullptr);
	BIO_free(inputBuffer);

	if (rsa == nullptr)
		throw AException("Could not create x509 certificate");

	return _<AX509>(new AX509(rsa));
}
