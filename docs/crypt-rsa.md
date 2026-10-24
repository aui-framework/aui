# RSA keys and encryption

[ARsa] holds an RSA key and encrypts or decrypts data with it. Instances are created through static factories and are
returned as `_<ARsa>`.

## Generating a key pair

```cpp
#include <AUI/Crypt/ARsa.h>

auto keys = ARsa::generate();      // 2048 bits
auto big = ARsa::generate(4096);
```

Generation of large keys takes noticeable time; do it off the UI thread.

## Exporting and importing keys

Keys are exchanged in PEM format. `getPrivateKeyPEM()` and `getPublicKeyPEM()` return the PEM text as a byte buffer, so
it can be written to a file directly. The private key is written unencrypted.

```cpp
AFileOutputStream("private.pem") << keys->getPrivateKeyPEM();
AFileOutputStream("public.pem") << keys->getPublicKeyPEM();
```

`ARsa::fromPrivateKeyPEM` and `ARsa::fromPublicKeyPEM` read them back. Both throw `AException` on malformed input.

<!-- aui:snippet aui.crypt/tests/Rsa.cpp rsa_private_roundtrip -->

## Sending data to someone who has only the public key

The holder of the public key encrypts, the holder of the private key decrypts:

<!-- aui:snippet aui.crypt/tests/Rsa.cpp rsa_public_only -->

(In this test the same instance is used for decryption; in a real application the private key stays on the other side.)

## Data size

`encrypt` accepts input of any length. It is split into chunks of `getKeyLength() - 11` bytes (PKCS#1 v1.5 padding),
and each chunk becomes a `getKeyLength()` byte block of the output. The output is therefore larger than the input, and
`decrypt` expects exactly the concatenation of such blocks.

RSA is slow and the output grows, so for bulk data it is usually used to protect a small secret rather than the data
itself. This module has no symmetric cipher; bring your own.

!!! warning

    `ARsa` offers encryption only. It does not sign or authenticate data, and it uses PKCS#1 v1.5 padding. Review
    whether this fits your protocol before relying on it.

Errors in encryption or decryption (for example, decrypting corrupted or foreign data) throw `AException`.
