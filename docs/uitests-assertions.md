# Checking views

`UIMatcher::check` applies an assertion to every matched view. The assertion is a function object that takes a view and
returns `bool`; the second argument of `check` is the message shown when it fails.

```cpp
By::name("#hello").check(visible(), "label is not appeared")
                  .check(text("Hello, Steve!"), "invalid text");
```

Failures are non-fatal (`EXPECT_*`): the test goes on and reports every problem. Empty selection fails the check, so a
typo in a selector does not make the test pass silently. `check` returns the matcher, so checks and actions can be
chained.

## Visibility and text

| Assertion      | Passes when                                                          |
|----------------|----------------------------------------------------------------------|
| `visible()`    | the view is rendered (`Visibility::VISIBLE`)                         |
| `notVisible()` | the view is not rendered (`INVISIBLE` or `GONE`)                     |
| `gone()`       | `Visibility::GONE`: not rendered and occupies no space               |
| `text("...")`  | the text of the view equals the string                               |

These visibility assertions are the ones that see invisible views; every other assertion works on visible views only.

The following test checks that the label stays hidden until the button is clicked:

<!-- aui:snippet aui.uitests/tests/UIClickTest.cpp click_test -->

To check something that has no assertion (a state of a view), cast the view and use the GTest macros:

```cpp
auto field = _cast<ATextField>(By::name("#username").one());
ASSERT_TRUE(field);
EXPECT_EQ(field->getText(), "Steve");
```

## Sizes

`width(metric)` and `height(metric)` compare with an exact size. `widthIsMinimal()` and `heightIsMinimal()` check that
the view is not larger than its minimum size. `sameWidth()` and `sameHeight()` check that all matched views have equal
sizes, whatever it is.

<!-- aui:snippet aui.uitests/tests/UILayoutTest.cpp expanding_width -->

<!-- aui:snippet aui.uitests/tests/UILayoutTest.cpp same_width -->

Sizes in `width(...)` and `height(...)` accept `_dp`, so tests do not depend on the screen density.

## Alignment and order

`areLeftAligned()`, `areRightAligned()`, `areTopAligned()`, `areBottomAligned()`, `areLeftRightAligned()` and
`areTopBottomAligned()` check that all matched views share the edge (or both edges). The assertion remembers the first
view's edge, so the check is relative: the views are aligned to each other.

`isTopAboveTopOf(matcher)`, `isTopBelowTopOf(matcher)`, `isBottomAboveBottomOf(matcher)` and
`isBottomBelowBottomOf(matcher)` compare an edge with the same edge of all views matched by another matcher. That is the
way to check if something is scrolled out of an area:

<!-- aui:snippet aui.uitests/tests/UIScrollTest.cpp scroll -->

`isParentOf(view)` passes for the matched views that contain `view` somewhere inside.

```cpp
By::type<AScrollArea>().check(isParentOf(label));
```

## Negation

`uitest::impl::not$(assertion)` inverts an assertion. It lives in the `impl` namespace, but it is the way to say "not":

```cpp
By::text("Content").check(uitest::impl::not$(isTopAboveTopOf(By::type<AScrollArea>())));
```

## Colors

`averageColor(color, inaccuracy)` and `pixelColorAt({ x, y }, color, inaccuracy)` look at the rendered pixels. See
[uitests-screenshots].

## Custom assertions

Any callable `bool(const _<AView>&)` is an assertion. It may use GTest macros inside, which gives much better
messages than a bare `false`:

<!-- aui:snippet aui.uitests/tests/UITextFieldTest.cpp custom_assertion -->

Use it as `By::type<ATextField>().check(selectionMatches(0, 5))`.

By default, an assertion is evaluated on visible views only. If it should see invisible ones (like `notVisible()`
does), add `using IGNORE_VISIBILITY = std::true_type;` to the assertion type.

Matchers can be extended in the same way: derive from `IMatcher`, implement `matches(const _<AView>&)` and wrap it in
[UIMatcher]:

```cpp
class HasTooltip : public IMatcher {
public:
    bool matches(const _<AView>& view) override {
        return view->getAssNames().contains(".has-tooltip");
    }
};
auto matcher = UIMatcher(_new<HasTooltip>());
```
