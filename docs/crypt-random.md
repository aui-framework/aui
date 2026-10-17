# Random bytes

`ACrypto::safeRandom()` (`AUI/Crypt/Random.h`) returns an `_<AByteBuffer>` of 4096 cryptographically secure random
bytes. They are produced by a CTR_DRBG generator (mbedTLS) which is seeded from the system entropy source on every
call, so it is suitable for key material, tokens and nonces.

```cpp
#include <AUI/Crypt/Random.h>

_<AByteBuffer> bytes = ACrypto::safeRandom();
```

The function throws `AException` if the generator can't be seeded. Seeding makes each call relatively expensive; if you
need a lot of random numbers, call it once and take what you need from the buffer.

For non-security randomness (shuffling, test data), use `ARandom` from aui::core.

Key generation in [ARsa] does not use this function; it seeds its own mbedTLS generator from the same entropy source.

See also [crypt] for the module overview.
