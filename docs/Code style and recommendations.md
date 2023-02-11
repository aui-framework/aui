# Code style
AUI's code should be kept with the following code style:
- `No tabs`; only four spaces
- Row length: up to `120` characters
- Class names: `CamelCase`. Every framework public API class name starts with capital '`A`'. Use `GenericSpecific` pattern
  (`EventClose`, not `CloseEvent`, `PacketConnect`, not `ConnectPacket`). Using that convention files group nicely in
  file lists.
- Functions, variables: `lowerCamelCase`
- Constants: `UPPER_SNAKE_CASE`
- Member fields: `m...` (`lineNumber` -> `mLineNumber`)
- Getters: `...` (`lineNumber` -> `lineNumber()`, (field: `mLineNumber` )
- Getters: `set...`/`with...` (`lineNumber` -> `setLineNumber(...)`, `accessible` -> `withAccessible(...)`)
- Structures: commonly holds some data and does not have member functions; `CamelCase`.
  Also used for stl-like functionality (AUI/Traits), in that case, `snake_case` used
  for file names, struct name and its member functions.
- Constructors and setters: move semantics. This allows caller to choose whether copy or move the passed object
- Use `const`, `noexcept` and `[[nodiscard]]` whenever possible
- Tend to not to use macros. `UPPER_SNAKE_CASE` prefixed with '`AUI_`'. If the macro is intended to efficiently reduce code size (i.e. `ui_thread` or `async`) it's allowed to consciously name it using `lower_snake_case` without prefix.
- Use `#pragma once` instead of C-style include guards
- Use Doxygen (`@`-style, not `\`)
- Avoid global functions (at least put them to namespaces)
- Every symbol (class, struct, namespace, file) prefixed with '`A`', '`aui`', '`AUI_`' is a part of AUI's stable public API ready for the external usage. Symbols without such prefixes are called _internal_ and can be also used (if possible) when needed. Be careful using internal symbols since
  there's no guarantee about their API stability.

Basic example:
```cpp
class User {
private:
  AString mUsername;

public:
  // User(const AString& username): mUsername(username) {} <--- outdated
  // use this instead:
  User(AString username) noexcept: mUsername(std::move(username)) {}

  [[nodiscard]
  const AString& username() const noexcept { // a typical getter; notice the const and noexcept keywords and [[nodiscard]] attribute
    return mUsername;
  }

  void setUsername(AString username) noexcept { // a typical setter; also uses noexcept
    mUsername = std::move(username);
  }
};
```


# Assertions

The whole AUI framework's code filled with assertion checks so if you do something wrong the framework will tell you
about it. Also in AUI almost every assertion contains a quick tip how to solve the problem. It is recommended to you to
do the same. For example:

```cpp
assert(("AAbstractThread::processMessages() should not be called from other thread",
        mId == std::this_thread::get_id()));
```

The code above ensures that the function was not called from some other thread. As you can see, the tooltip is produced
using extra brace pair and `operator,`:

```cpp
assert(("your message", expression))
```

Do not put algorithm-necessary code inside `assert()` since asserts are removed in release builds on some compilers, i.e. don't `assert(("someAction failed!", someObject->someAction() != 0))` since it leads to hard-to-find bugs.

## Assert or exception?

Assert is an enemy for the production application since it *terminates* program execution. Use it when it's condition relies only on the developer.
Quick example:

```cpp
connect(mLoginButton->clicked, me::loginButtonClicked);
...
void loginButtonClicked() {
  assert(mUsername->text().length() < 32); // bad! throw an exception instead so it can be handled: throw AException("username is too long!")
}
```

# STL-like functionality

It's hard to say which functionality can be called 'STL-like'. Commonly, any iterator-based algorithm (i.e. `aui::binary_search`), global functions, trait structs are STL-like functionality. The final goal is to avoid mixed-style expressions like `AString::const_iterator` which hurts the eyes.

# Template metaprogramming and macros

Both C++ template instantiation mechanism and macro preprocessor are Turing complete. However, writing and understanding
C++ template metaprogramming (TMP) and macro preprocessor code requires expert knowledge of C++ and a lot of time to
understand. Use TMP deliberately.

Since TMP and macros often evolve custom syntax and usage scenarios, consider writing especially well documentation with
examples when defining public API templates and macros.