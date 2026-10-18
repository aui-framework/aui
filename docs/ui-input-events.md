# Handling input events

Views report what the user does through signals and virtual functions. For ordinary reactions, connect to a signal; for
custom views, override the function.

## Reacting to a click

[AView] has the `clicked` signal, which every view emits when the user presses and releases the left mouse button
(or taps) inside of it. A [AButton] is not special in this regard: a label or any other view can be clicked as well.

```cpp
_new<ALabel>("Open website") AUI_LET {
    AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [] {
        APlatform::openUrl(AUrl("https://github.com/aui-framework/aui"));
    });
}
```

Related signals of [AView]:

| Signal                         | Emitted when                                                  |
|--------------------------------|---------------------------------------------------------------|
| `clicked`                      | left button (or tap) released over the view                   |
| `clickedButton`                | any button released over the view; carries `APointerIndex`    |
| `clickedRight`                 | right button released over the view                           |
| `clickedRightOrLongPressed`    | right click, or long press on touchscreens                    |
| `doubleClicked`                | double click                                                  |
| `pressed` / `released`         | pointer pressed / released                                    |
| `mouseEnter` / `mouseLeave`    | pointer enters / leaves the view                              |
| `scrolled`                     | mouse wheel or touch scrolling                                |
| `keyPressed` / `keyReleased`   | keyboard key pressed / released while the view is focused     |
| `focusAcquired` / `focusLost`  | focus changes                                                 |

To click a view from code (i.e., a hotkey that does the same as a button), call `AView::click()`; it emits `clicked`.

For buttons specifically, see the examples on the [abutton] page.

## Pointer events in a custom view

[AView] offers `onPointerPressed`, `onPointerReleased`, `onPointerMove`, `onPointerDoubleClicked` and `onScroll`. A view
that is pressed continues to receive move and release events even if the pointer leaves it, so dragging works out of
the box. Use `AView::isMouseHover()` to check whether the pointer is actually over the view.

Coordinates in the events are relative to the view's top left corner, in pixels.

```cpp
void onPointerReleased(const APointerReleasedEvent& event) override {
    AView::onPointerReleased(event);   // always call the base implementation to keep `clicked` working
    if (event.asButton != AInput::LBUTTON) {
        return;
    }
    addPoint(event.position);
}
```

!!! note

    Pointer events are unified for mouse and touchscreen. A finger touch is reported as a left button press;
    `event.pointerIndex` tells which finger or which mouse button caused the event.

A scroll event carries `event.origin` (where the pointer is) and `event.delta` (how much was scrolled). The fractal
example uses them to zoom around the pointer.

## Keyboard

Keyboard events go to the focused view and then bubble up through its parents. A view overrides:

- `onKeyDown(AInput::Key)` - a key was pressed (not a character: arrows, function keys, etc);
- `onKeyRepeat(AInput::Key)` - the key is held and the OS autorepeat fires;
- `onKeyUp(AInput::Key)`;
- `onCharEntered(AChar)` - a character was typed, with layout, shift and input methods already applied. Use this
  for text.

<!-- aui:snippet examples/app/fractal/src/FractalView.cpp key_handling -->

Keys are values of the `AInput::Key` enum. `AInput::isKeyDown(key)` polls the current state of a key anywhere in the
code, which is handy for modifiers:

```cpp
if (AInput::isKeyDown(AInput::LSHIFT)) {
    // ...
}
```

`AInput::CMD_CTRL` maps to ++cmd++ on Apple platforms and to ++ctrl++ elsewhere, which is what you want for hotkeys
like copy or save.

### Keyboard shortcuts

AUI does not have a global hotkey registry; a shortcut is a key press handled by a view. A window is a view container
as well, so the simplest place for application-wide shortcuts is the window itself:

```cpp
class MainWindow : public AWindow {
public:
    void onKeyDown(AInput::Key key) override {
        AWindow::onKeyDown(key);
        if (AInput::isKeyDown(AInput::CMD_CTRL) && key == AInput::S) {
            save();
        }
    }
};
```

`AShortcut` (built with `AInput::CMD_CTRL + AInput::S`) is only a description which converts to a readable string like
`Ctrl+S`; it is used for menu items, see [AMenu].

### Intercepting a key before the text field

A key that is not consumed by the focused view bubbles up: for instance, ++enter++ typed in an [ATextField] reaches the
`onKeyDown` of its parents. Overriding `onCharEntered` is the way to change what a text view does with typed
characters. The notes example turns ++enter++ in the title into "go to the next field":

```cpp
class TitleTextArea : public ATextArea {
public:
    using ATextArea::ATextArea;
    void onCharEntered(AChar c) override {
        if (c == '\r') {
            ASurface::current()->focusNextView();
            return;
        }
        ATextArea::onCharEntered(c);
    }
};
```

## Focus

Only one view of a window has focus. Views that handle keyboard input take it when clicked or when the user tabs to
them. In code:

- `view->focus()` gives the view focus (a freshly shown text field typically wants this);
- `view->isFocused()` / `view->hasFocus()` query the state, `focusAcquired` / `focusLost` / `focusState` signals
  report changes;
- `ASurface::focusNextView()` moves focus like ++tab++ does.

The focused state can be used in [ass] selectors to highlight the view.

## Mouse cursor

The shape of the pointer over a view is set with `AView::setCursor` or the cursor ASS property, which takes
[ACursor]:

```cpp
Label { "link" } AUI_OVERRIDE_STYLE { ACursor::POINTER }
```

Besides the system cursors (`ACursor::POINTER`, `TEXT`, `MOVE`, `EW_RESIZE`, `NS_RESIZE`), a cursor can be built from an
image or a drawable.

## Context menu

Override `AView::composeContextMenu()` to return a menu model that is shown on right click or long press. See
[ui-dialogs-and-menus].

## Testing input

UI tests simulate clicks, key presses and typing against a real window; see [writing-tests] and the `aui.uitests`
module (`By::type<ATextField>().perform(click())`, `keyDownAndUp(AInput::RETURN)`, `type("text")`).
