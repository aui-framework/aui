# Screenshots and pixels

The software renderer used by UI tests is deterministic, so what is drawn can be verified in a test. It is the last
resort for custom drawing and styles; for layout and state, prefer [uitests-assertions].

## Colors of a view

`averageColor(color, inaccuracy)` takes a screenshot, clips it to the view and compares the average color. `inaccuracy`
is in `[0;1)`, where `0` demands the same color and `1` matches anything; the default is `0.1`.

<!-- aui:snippet aui.uitests/tests/UIScreenAnalyzerTest.cpp average_color -->

`pixelColorAt({ x, y }, color, inaccuracy)` checks one pixel. The position is relative to the view and within `[0;1]`:
`{ 0, 0 }` is the top-left corner, `{ 1, 1 }` is the bottom-right, `{ 0.5f, 0.5f }` is the center.

<!-- aui:snippet aui.uitests/tests/UIStyleTest.cpp pixel_color -->

Pixel checks are the way to test [ass] rules: set an extra stylesheet, check the color.

## Custom analysis

[ScreenshotAnalyzer] gives access to the image. `makeScreenshot()` captures the window, `clip(view)` or
`clip(position, size)` crops it, `averageColor()` and `image()` return the results.

```cpp
auto shot = ScreenshotAnalyzer::makeScreenshot().clip(By::name("#preview").one());
EXPECT_EQ(shot.image().size(), glm::uvec2(64, 64));
```

## Seeing what the test sees

When a test fails, `testing::UITest` writes screenshots to the `reports/` directory (relative to the working directory
of the test run):

- `reports/<suite>/fail-<test>.png` for every failed assertion, with the matched views outlined in red;
- `reports/<suite>/finish-<test>.png` for the final state of every test.

To look at an intermediate state, call `saveScreenshot("name")` from the test body. The file is placed in `reports/`
and named `<suite>.<test>_<name>.png`.

```cpp
TEST_F(UIDataBinding, Dropdown) {
    saveScreenshot("initial");
    By::type<ADropdownList>().perform(click());
    saveScreenshot("opened");
}
```

!!! tip

    Upload `reports/` as an artifact in your CI, so a red build comes with the pictures.
