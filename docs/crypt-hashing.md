# Hashing and HMAC

`AHash` provides message digests and HMAC. All functions return the raw digest as an [AByteBuffer]; use
`toHexString()` or `toBase64String()` to get text.

| Function               | Digest size |
|------------------------|-------------|
| `AHash::md5`           | 16 bytes    |
| `AHash::sha1`          | 20 bytes    |
| `AHash::sha256`        | 32 bytes    |
| `AHash::sha512`        | 64 bytes    |
| `AHash::sha256hmac`    | 32 bytes    |

!!! warning

    MD5 and SHA-1 are broken for security purposes. Use them only to interoperate with existing formats.

## Hashing a buffer

```cpp
#include <AUI/Crypt/AHash.h>

AByteBuffer digest = AHash::sha256(AByteBuffer::fromString("hello"));
ALogger::info("Sample") << digest.toHexString();
```

## Hashing a file or other stream

Every digest has an overload accepting an input stream. The stream is read until its end, so there is no need to load
the whole file in memory:

```cpp
auto checksum = AHash::sha256(AFileInputStream("big.iso")).toHexString();
```

The overload takes `aui::no_escape<IInputStream>`, so it accepts a stream object (temporary or lvalue), a pointer or a shared pointer
(for example `_new<AByteBufferInputStream>(buffer)`).

## Verifying a download

Compare the hex digest with the published one. Hex output is lowercase.

```cpp
bool matches = AHash::sha256(AFileInputStream(path)).toHexString() == expectedSha256;
```

## HMAC

`AHash::sha256hmac(in, key)` computes HMAC-SHA256 of `in` with a secret `key`. It is used to authenticate a message
with a shared secret (webhook signatures, signed cookies, etc...):

```cpp
auto signature = AHash::sha256hmac(AByteBuffer::fromString(body), AByteBuffer::fromString(secret));
if (signature.toHexString() != receivedSignature) {
    throw AException("bad signature");
}
```

!!! note

    The `==` comparison above is not constant-time. Whether that matters depends on your threat model.

## Passwords

A plain digest, even salted, is a poor way to store passwords: this module does not implement a password hashing
function (bcrypt, scrypt, PBKDF2, argon2).

See also [crypt] for the module overview.
