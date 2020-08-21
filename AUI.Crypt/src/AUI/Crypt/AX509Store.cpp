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
