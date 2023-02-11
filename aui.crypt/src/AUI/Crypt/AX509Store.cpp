// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
