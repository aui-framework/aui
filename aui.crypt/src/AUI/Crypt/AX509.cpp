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

_<AX509> AX509::fromPEM(AByteBufferView buffer)
{
	BIO* inputBuffer = BIO_new_mem_buf(buffer.data(), buffer.size());
	X509* rsa = nullptr;
	PEM_read_bio_X509(inputBuffer, (X509**)&rsa, nullptr, nullptr);
	BIO_free(inputBuffer);

	if (rsa == nullptr)
		throw AException("Could not create x509 certificate");

	return aui::ptr::manage(new AX509(rsa));
}
