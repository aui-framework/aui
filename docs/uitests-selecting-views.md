# Finding views

Every UI test starts with finding the views to work with. `By` is a set of functions that return a [UIMatcher]; the
matcher is then used to `perform` actions and `check` assertions. Matching is lazy: the view tree is searched on every
call, so a matcher stays valid after the UI has changed.

## By text, name and type

`By::text` matches views whose text equals the string: labels, buttons, text fields, anything that converts to a string
via `IStringable` (so `AButton` is matched by its caption).

```cpp
By::text("Say hello").perform(click());
```

`By::name` matches by ASS name, which is assigned with `<<` in the view tree (`"#username"`, `".container"`) or with
`addAssName`. A name is the most robust selector: it does not depend on a text that may be translated or changed.

```cpp
mTextField = _new<ATextField>() << "#username",
...
By::name("#username").perform(type("Steve"));
```

`By::type<T>()` matches views of type `T` or derived from it, and `By::exactType<T>()` only of exactly `T`.

```cpp
By::type<AButton>().check(sameWidth(), "widths of the buttons are not equal");
```

`By::value(view)` matches a particular view object, which is handy for combining it with other matchers.

!!! note

    Only visible views are matched by default. A view with `Visibility::INVISIBLE` or `Visibility::GONE`, and
    everything inside it, is skipped. Assertions `visible()`, `notVisible()` and `gone()` are exceptions; they look at
    invisible views, too. Call `includeInvisibleViews()` to find invisible views with `toSet()`, `one()`, `perform` or `check`.

## Combining matchers

`|` selects views matched by any of the operands, `&` by both:

```cpp
(By::text("Common button") | By::text("Show all views...")).check(sameHeight());
(By::type<AButton>() & By::name(".primary")).perform(click());
```

Mind that `perform` is applied to every matched view, in unspecified order. When you need exactly one view, make the
selector precise enough, or use `one()`.

## Getting the view itself

`one()` returns the first matched view as `_<AView>`, or `nullptr` if there is none. `toSet()` and `toVector()` return
all of them. Cast to the concrete type to inspect it:

```cpp
auto label = _cast<ALabel>(By::name("#hello").one());
ASSERT_TRUE(label);
EXPECT_EQ(*label->text(), "Hello, Steve!");
```

Position and size are in `getPositionInWindow()`, `getSize()` and `getCenterPointInWindow()`; these are what the
`click` action uses to aim.

## Relatives

`parent()` selects containers that have a direct child matched by this matcher, `allChildren()` selects views whose
parent is matched. Together they select siblings:

```cpp
// every element in the same container as the "Common button"
By::name("Common button").parent().allChildren().check(areLeftAligned());
```

## Finding the nearest view

Forms usually pair a label with a field. `findNearestTo` picks, among the matched views, the one closest to another
view, measured from its bottom-right corner (the way a person scans a form). The argument must match exactly one view.

```cpp
// layout: Label "Login", ATextField, Label "Password", ATextField
By::type<ATextField>().findNearestTo(By::text("Login")).perform(type("alice"));
By::type<ATextField>().findNearestTo(By::text("Password")).perform(type("secret"));
```

## Many windows

A matcher looks into all windows opened by the window manager, so dialogs and popups are found without extra effort.
Make the selector narrow enough if the same text appears in several windows.
