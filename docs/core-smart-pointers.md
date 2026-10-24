# Smart pointers

AUI objects are normally owned by shared pointers. This page describes the pointer types and the shortcuts around them.

## AArc, AWeakArc and AUnique

| Type                | Based on          | Notes                                                    |
|---------------------|-------------------|----------------------------------------------------------|
| [aui::Arc], `AArc<T>`          | `std::shared_ptr` | "atomic reference counter"                    |
| [aui::WeakArc], `AWeakArc<T>`  | `std::weak_ptr`   | `lock()` returns an `AArc`                    |
| `AUnique<T>`                   | `std::unique_ptr` |                                               |

They inherit from the standard types, so they work with any API expecting `std::shared_ptr` and so on.

The `_<T>`, `_weak<T>` and `_unique<T>` aliases are older, shorter names of the same types. You'll see them all over the
examples, but the header marks them as deprecated in favor of the `A`-prefixed spelling.

```cpp
_<MyClass> a;     // same as AArc<MyClass>
```

## Creating objects

`_new<T>(args...)` is `std::make_shared` that returns an [aui::Arc]:

```cpp
auto view = _new<AButton>("Click me");
```

An [aui::Arc] cannot be constructed from a raw pointer by accident: that constructor is deleted. Say how the memory is
managed instead:

```cpp
auto owned = aui::ptr::manage_shared(new Legacy());   // delete is called when the last reference dies
auto notOwned = aui::ptr::fake_shared(&someGlobal);   // destructor does nothing
```

Use `fake_shared` when an API demands a shared pointer but your object lives on the stack or in a static.

## Setting up an object in place

Creating an object and configuring it with a few setters usually takes a variable and several lines. Two helpers let
you keep it in one expression. `AUI_LET` runs a block right after the expression, with the object named `it`:

<!-- aui:snippet aui.core/tests/SharedPtrsTest.cpp core_ptr_let -->

`AUI_PERFORM_AS_MEMBER` makes the block behave as a member function of the object, so there is no need to repeat its
name:

<!-- aui:snippet aui.core/tests/SharedPtrsTest.cpp core_ptr_perform_as_member -->

[aui::Arc] also has a `connect` builder, which makes it possible to wire up signals without leaving the expression:

<!-- aui:snippet aui.core/tests/SharedPtrsTest.cpp core_ptr_connect_builder -->

See [signal_slot] for what signals and slots are.

## Weak references

`weak()` makes an [aui::WeakArc]; `lock()` gives back an [aui::Arc], which is empty if the object is gone. Use it to avoid
reference cycles and to keep an async callback from extending the lifetime of an object.

```cpp
void MyWindow::watch(const _<MyView>& view) {
    mCallback = [weakView = view.weak()] {
        if (auto view = weakView.lock()) {
            view->setVisibility(Visibility::GONE);
        }
    };
}
```

## `this` as a shared pointer

[AObject] derives from `std::enable_shared_from_this`. Calling `shared_from_this()` on a derived class returns a
pointer to the base, so AUI provides `aui::ptr::shared_from_this(this)` and `aui::ptr::weak_from_this(this)` that return
a pointer of the right type:

```cpp
auto self = aui::ptr::shared_from_this(this);   // AArc<MyView>
auto weakSelf = aui::ptr::weak_from_this(this); // AWeakArc<MyView>
```

## Pointer to a member of a shared object

`AUI_PTR_ALIAS(owner, field)` returns a shared pointer to `owner->field` that keeps the whole `owner` alive. It's
useful to pass a part of a state object to some component without the component knowing about the entire state.

```cpp
struct State {
    AProperty<bool> checked;
};

_<State> state = _new<State>();
_<AProperty<bool>> checked = AUI_PTR_ALIAS(state, checked);
```

## Casting

`_cast<To>(ptr)` is `dynamic_pointer_cast` for [aui::Arc]:

```cpp
if (auto button = _cast<AButton>(view)) {
    // view is actually a button
}
```
