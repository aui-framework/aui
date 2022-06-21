# `AUI_BUILD_EXAMPLES`

Whether build examples or not.

# `AUI_INSTALL_RUNTIME_DEPENDENCIES`
When `true`, all AUI's dependencies will be installed along with it. **WARNING** on Linux doing `make install` with `AUI_INSTALL_RUNTIME_DEPENDENCIES=TRUE` is dangerous and may cause system instability. It will replace some system libraries with AUI's ones (i.e. `libssl`, `libcrypto`, `GLEW`, etc...) which may produce the following messages:

```
libssl.so.1.0.0: no version information available (required by openssl)
```

# `AUI_DISABLE_HIDPI`

Disables HIDPI.

# `AUI_SHARED_PTR_FIND_INSTANCES`

Adds `printAllInstances()` to AUI's shared pointer type (`_`) which prints stacktrace from constructor of every instance of `shared_ptr` (`_`) pointing to that object. Made for debugging purposes to find cycle and unwanted pointers. Dramatically slows the application's performance.