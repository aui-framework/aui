# Simulating user input

Actions are objects that are passed to `UIMatcher::perform`. Each action is applied to every matched view, and the UI
is processed (`uitest::frame()`) after each step, so a handler's reaction is visible immediately. Input goes through
the same window entry points the platform uses (`onPointerPressed`, `onCharEntered`, `onKeyDown` and so on), so the
views see what they would see from a real user.

```cpp
By::text("Say hello").perform(click());
```

## Clicks

`click()` presses and releases the left button at the center of the view. `clickRight()` does the same with the right
button.

<!-- aui:snippet aui.uitests/tests/UIClickTest.cpp right_click -->

Pass a position relative to the view's top-left corner to click somewhere else. This is how the text field tests
place a cursor:

<!-- aui:snippet aui.uitests/tests/UITextFieldTest.cpp click_position -->

`doubleClick()` takes an optional position, too:

<!-- aui:snippet aui.uitests/tests/UITextFieldTest.cpp double_click -->

`gestureLongPress()` emits a long press gesture, the touch counterpart of the right click:

<!-- aui:snippet aui.uitests/tests/UIClickTest.cpp long_press -->

For pointer sequences, use the separate steps: `mousePress()`, `mouseRelease()` and `mouseMove()` (and
`mousePressRight()`, `mouseReleaseRight()`). `pointerPress`, `pointerRelease` and `pointerMove` are there for the cases
when a pointer index has to be specified.

If you need a pointer event that the actions do not cover, call the window directly, as `UIClickTest` does:

```cpp
mTestWindow->onPointerPressed({ .position = view->getCenterPointInWindow(), .asButton = AInput::LBUTTON });
```

## Typing

`type("text")` focuses the view and enters the string character by character, as if typed on the keyboard. It does not
clear the field; select all first if you want to replace the contents.

<!-- aui:snippet aui.uitests/tests/UITypeTest.cpp type_test -->

## Key presses

`keyDownAndUp(key)` focuses the view and presses and releases a key. The argument is an `AShortcut`, so combinations are
formed with `+`:

<!-- aui:snippet aui.uitests/tests/UITextFieldTest.cpp ctrl_a -->

Actions can be chained on the same matcher, interleaved with checks. The following test moves the cursor with the
keyboard and checks its position after each step (`selectionMatches` is a custom assertion, see
[uitests-assertions]):

<!-- aui:snippet aui.uitests/tests/UITextFieldTest.cpp key_press -->

## Scrolling and hovering

`scroll({ dx, dy })` delivers a scroll event to the view. `mouseMove()` moves the pointer to the view (with an
optional offset), which updates hover state.

<!-- aui:snippet aui.uitests/tests/UIScrollTest.cpp scroll -->

To scroll a particular child into view, call `AScrollArea::scrollTo(view)` directly; actions are only needed when the
scroll itself is what you test.

## Summary

| Action                               | What it does                                            |
|--------------------------------------|---------------------------------------------------------|
| `click()`, `clickRight()`            | Press and release the left or right button              |
| `doubleClick()`                      | Two clicks and a double click event                     |
| `gestureLongPress()`                 | Long press gesture                                      |
| `mousePress()`, `mouseRelease()`     | Press or release the left button                        |
| `mouseMove()`                        | Move the pointer                                        |
| `type("...")`                        | Enter characters into the focused view                  |
| `keyDownAndUp(key)`                  | Press and release a key or a shortcut                   |
| `scroll({ dx, dy })`                 | Mouse wheel                                             |

Position arguments are relative to the view's top-left corner and accept `_dp` literals, as in
`click({ 23_dp, 0_dp })`.
