# (7GUIs) Flight Booker

@auiexample{7guis}
Flight Booker

# Regex Library

For validation in this example, we've chosen using regex technology, as it's fairly simple and extensible way to
make parsers.

Despite STL provides regex implementation, it varies from compiler to compiler, compiles the regex expression at runtime
only, and some platforms may even lack builtin regex library. To avoid possible issues, custom implementation should be
used.

Although AUI does not provide a regex parser on its own, nothing stops you from @ref AUI_BOOT_3RDPARTY "using AUI.Boot"
in order to pull awesome 3rdparty implementation of your choice that suits your exact needs. For this example, we've
chosen [ctre](https://github.com/hanickadot/compile-time-regular-expressions), as it evaluates the regex expression at
compile-time, emitting effective code, as if we were validating the string manually.
