/*
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "AX509Store.h"
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <cassert>
#include "AUI/Common/AException.h"

AX509Store::AX509Store()
{
	mX509Store = X509_STORE_new();
	assert(mX509Store);
}

AX509Store::~AX509Store()
{
	X509_STORE_free((X509_STORE*)mX509Store);
}

bool AX509Store::validate(_<AX509> cert)
{
	auto ctx = X509_STORE_CTX_new();
	assert(ctx);
	X509_STORE_CTX_init(ctx, (X509_STORE*)mX509Store, (X509*)cert->mX509, nullptr);
	int r = X509_verify_cert(ctx);
	X509_STORE_CTX_free(ctx);

	if (r == -1)
	{
		throw AException("Could not validate certificate");
	}

	return r;
}

void AX509Store::addCert(_<AX509> cert)
{
	mCertificates << cert;
	X509_STORE_add_cert((X509_STORE*)mX509Store, (X509*)cert->mX509);
}
