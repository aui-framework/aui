# Your first UI test

This page shows how to set up a UI test suite and run a complete scenario. For the general test setup
(`aui_enable_tests`, running `Tests`), see [writing-tests]. An overview of the module is at [uitests].

## How it works

A UI test creates real windows and views, but no native window is opened. `testing::UITest` switches to
[AStubWindowManager] and a software renderer, so the test runs on a machine without a display. The views are laid out,
styled and rendered as usual, the screenshot of any moment is available, and input is delivered to the window the same
way the platform would do it.

Include `<AUI/UITest.h>`; it brings `By`, all actions, all assertions and GTest.

## Fixture

Derive your suite from `testing::UITest`. Create and show the window in `SetUp`, and call the base class methods.

<!-- aui:snippet aui.uitests/tests/UIClickTest.cpp click_fixture -->

The application under test here is a button "Say hello" that reveals a "Hello!" label. Note `GMock`: the window
exposes `MOCK_METHOD`s that the signal handlers call, so the test can tell whether the click was delivered.

!!! tip

    Place the window in the fixture class as a nested class, as above, when it is only needed for tests. To test a
    window of your application, include its header and `_new<MyWindow>()->show()` it.

## Test body

Find a view, perform an action, check the outcome:

<!-- aui:snippet aui.uitests/tests/UIClickTest.cpp click_test -->

`By::text("Say hello")` finds the button, `click()` clicks the center of it, and `By::text("Hello!")` finds the label
and `visible()` checks that it is shown. See [uitests-selecting-views], [uitests-actions] and [uitests-assertions].

## Typing and reading values

Views can be found by their ASS name (`<< "#username"` in the view tree). The following window has a text field and a
button; the test types a name, presses the button and checks the label.

<!-- aui:snippet aui.uitests/tests/UITypeTest.cpp type_fixture -->

<!-- aui:snippet aui.uitests/tests/UITypeTest.cpp type_test -->

## Processing the UI

A UI is not updated by itself in a test; nothing runs an event loop for you. Layout, style changes and queued messages
(for example, lambdas posted with `AUI_UI_THREAD`) are processed when you call
`uitest::frame()`. Every `perform` and `check` of [UIMatcher] calls it before and after the action, so most tests
never need it. Call it explicitly when you changed something directly and want to look at the result:

```cpp
label->setVisibility(Visibility::GONE);
uitest::frame();
EXPECT_EQ(otherView->getPosition().x, 0);
```

`uitest::frame()` also repacks every window (see `AStubWindowManager::drawFrame`), so a window is shrunk to the size
of its contents unless the contents are expanding. Use `AUI_REPEAT(10) { uitest::frame(); }` to let a layout settle
if it needs several passes.

!!! note

    Timers and other asynchronous work do not advance on their own either. If your code waits for an `AThread::sleep`
    or a background task, call `AThread::processMessages()` after waiting, and prefer to make the code under test
    injectable (for instance, pass the delay as a parameter) so the test does not have to sleep.

## Testing data binding

Reactive UI is checked the same way: change the state and look at the views.

<!-- aui:snippet aui.uitests/tests/UIReactiveTest.cpp reactive_label -->

## Cleaning up

Release the windows and views you hold before calling `UITest::TearDown()`, as the fixtures above do
(`mTestWindow = nullptr;`). `UITest::TearDown()` closes all windows and destroys the window manager.
