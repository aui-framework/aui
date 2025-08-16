# Code Style and Recommendations

AUI's code should be kept with the following code style:

- `No tabs`; only four spaces (exception: for UI building, 2 spaces is acceptable)
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
- Tend to not to use macros. `UPPER_SNAKE_CASE` prefixed with '`AUI_`'
- Use `#pragma once` instead of C-style include guards
- Use Doxygen (`@`-style, not `\`)
- Avoid global functions (at least put them to namespaces)
- Every symbol (class, struct, namespace, file) prefixed with '`A`', '`aui`', '`AUI_`' is a part of AUI's stable public
  API ready for the external usage. Symbols without such prefixes are called _internal_ and can be also used (if
  possible) when needed. Be careful using internal symbols since there's no guarantee about their API stability.
- Follow visibility in this order when possible: `public`, `protected`, `private`. Header's user is more interested in
  public APIs rather than in private.

Basic example:
```cpp
class User {
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

private:
  AString mUsername;

};
```

!!! note

    With [property_system], a better way of defining data models will be:
    ```cpp
    struct User {
        AProperty<AString> username;
    };
    ```
    It allows aggregate initialization: `User u { .username = "Test" };`


## Assertions

The whole AUI framework's code filled with assertion checks so if you do something wrong the framework will tell you
about it. Also in AUI almost every assertion contains a quick tip how to solve the problem. It is recommended to you to
do the same. For example:

```cpp
AUI_ASSERTX(mId == std::this_thread::get_id(),
            "AAbstractThread::processMessages() should not be called from other thread");
```

The code above ensures that the function was not called from some other thread.

!!! note

    Do not put algorithm-necessary code inside `assert()`, `AUI_ASSERT` or `AUI_ASSERTX` since asserts are removed in
    release builds on some compilers, i.e. don't `assert(("someAction failed!", someObject->someAction() != 0))` since it
    leads to hard-to-find bugs.

### Assert or exception?

Assert is an enemy for the production application since it *terminates* program execution. Use it when it's condition relies only on the developer.
Quick example:

```cpp
connect(mLoginButton->clicked, me::loginButtonClicked);
...
void loginButtonClicked() {
  AUI_ASSERT(mUsername->text().length() < 32); // bad! throw an exception instead so it can be handled: throw AException("username is too long!")
}
```

## Code style exceptions

Commonly, any iterator-based algorithm (i.e. `aui::binary_search`), global functions, trait structs are STL-like
functionality. The final goal is to avoid mixed-style expressions like `AString::const_iterator` which hurts eyes.

## Template metaprogramming and macros

Both C++ template instantiation mechanism and macro preprocessor are Turing complete. However, writing and understanding
C++ template metaprogramming (TMP) and macro preprocessor code requires expert knowledge of C++ and a lot of time to
understand. Use TMP deliberately.

Since TMP and macros often evolve custom syntax and usage scenarios, consider writing especially well documentation with
examples when defining public API templates and macros.

## Improving compiler error messages techniques

### Try to break your templates

After considering actions listed below, try your types/traits/concepts against various awkward types/arguments/use 
cases.

### Concepts are preferable

Use concepts instead of SFINAE were possible.

### Raise static_assert messages

With `static_assert` with potentially helpful message, use `====================>` prefix in your message to raise your
message among a long list of compiler diagnostics.

<!-- aui:snippet aui.core/src/AUI/Reflect/AClass.h ARROW_ERROR_MESSAGE_EXAMPLE -->

### Single line comment error messages

You can exploit the fact that a compiler prints code lines in its diagnostics. Put a single line comment with long arrow
prefix to put potentially helpful messages. Use [TRY_TO_BREAK] technique to discover the lines to put the comments
in.

Cast failure example:

```cpp
auto& [a] = const_cast<std::remove_cv_t<Clazz>&>(clazz); // ====================> aui::reflect: Clazz is not a SimpleAggregate.
```

Overload substitution failure example:

```cpp
...
template<typename T>
requires requires(T& t) { std::hash<T>{}(t); }
constexpr std::size_t forEachKey(const T& value) { // ====================> std::hash based specialization
    return std::hash<T>{}(value);
}

template<typename T>
requires requires(T& t) { { t.base() } -> ranges::range; }
constexpr std::size_t forEachKey(const T& value) { // ====================> specialization for subranges
...    
```

Produces the following diagnostics:

```cpp
.../AForEachUI.h:220:92: error: no matching function for call to ‘forEachKey(...)’
220 |     return AForEachUIBase::Entry { .view = mFactory(t), .id = forEachKey(t) };
    |                                                               ~~~~~~~~~~^~~
...View/AForEachUI.h:34:23: note: candidate: template<class T> requires(T& t) {{}(t);} ...
34 | constexpr std::size_t forEachKey(const T& value) { // ====================> std::hash based specialization
```

This makes it obvious what does this overload do.

## .clang-format

clang-format is a de facto standard tool to auto format (C++) code with style described by `.clang-format` file that
typically located inside your project's root directory. AUI [has such file](CLANG_FORMAT).

Since AUI abuses C++'s syntax, it's important to set up appropriate auto formatting, or you will often find yourself
struggling with AUI's DSL especially in large portions of layout, despite the fact we recommend to decompose large
AUI DSLs into smaller pieces (i.e., functions). For your project, we recommend to start with AUI's formatting
configuration listed below. AUI's [App Template](https://github.com/aui-framework/example_app) has `.clang-format`
already. Your IDE should pick up it without further configuration.

### Always Use Trailing Comma in Initializer Lists

When it comes to `clang-format` there's one an unobvious feature when using AUI's DSL. Consider the example:

=== "With Trailing Comma"
    
    ```cpp
    setContents(Vertical {
      Button { "Up" },
      Button { "Down" },
    });
    ```
    
=== "Without Trailing Comma"
    
    ```cpp
    setContents(Vertical {
      Button { "Up" },
      Button { "Down" }
    });
    ```

See the difference? The second example lacks one comma. If we try to trigger `clang-format`
(<kbd>ALT</kbd>+<kbd>CTRL</kbd>+<kbd>L</kbd> in **CLion**), we'll get the following results (assuming AUI's
[CLANG_FORMAT] ".clang-format"):

=== "With Trailing Comma"
     
    ```cpp
    setContents(Vertical {
      Button { "Up" },
      Button { "Down" },
    });
    ```

=== "Without Trailing Comma"
    
    ```cpp
    setContents(Vertical {
      Button { "Up" }, Button { "Down" } });
    ```

The first example left as is (correct), the second example formatted confusingly.

When using any kind of AUI's DSL with initializer lists please always use trailing comma. Not only it helps with
reordering, git diffs, etc..., but also `.clang-format` makes proper formatting for lists with trailing commas.

### Use clang-format off/on

In some scenarios clang-format may fight against you, especially with complicated syntax. You can use
`// clang-format off` and `// clang-format on` to disable and enable clang-format, respectively.

<!-- aui:snippet aui.json/tests/JsonFieldsTest.cpp clang_format_toggle -->

### AUI's .clang-format

Place this `.clang-format` file in root of your project.

<!-- aui:include .clang-format -->
