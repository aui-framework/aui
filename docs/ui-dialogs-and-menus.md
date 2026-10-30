# Dialogs, menus and popups

AUI offers several ways to ask the user something, from OS-native modal boxes to windows and menus made of regular
views.

| Need                                   | Use                                       |
|----------------------------------------|-------------------------------------------|
| tell something or ask yes/no, urgently | `AMessageBox`                             |
| choose a file or a folder              | `ADesktop::browseForFile` / `browseForDir`|
| custom dialog with arbitrary contents  | a child [AWindow] with `WindowStyle::MODAL` |
| right click menu                       | `AView::composeContextMenu`, see [AMenu]  |
| list of options attached to a button   | [ADropdownList]                           |

## Message box

`AMessageBox` shows a native dialog. The call blocks until the user dismisses it and returns which button was pressed.

<!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp AMessageBox -->

Because of the blocking, it's better saved for things the application can't continue without. For anything else, show
the information inline.

`AMessageBox::show` can be used with a `nullptr` parent if there's no window around.

## File and folder choosers

`ADesktop::browseForFile` and `ADesktop::browseForDir` open the native dialog and return an `AFuture<APath>`. Do not
call a blocking getter on the future from the UI thread; subscribe with `onValue`. An empty path means the user
cancelled.

<!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp browse_file -->

The example keeps the future in an async holder (`mAsync`) owned by the window. The file dialog accepts a list of `ADesktop::FileExtension` (`{ "Images", "png" }`) as a filter.

## Custom dialog window

A dialog is just another [AWindow]. Pass the parent and `WindowStyle::MODAL` (no resizing, no minimize/maximize
buttons), fill it with views, and `show()` it. The circle drawer example opens such a window to adjust the radius of a
circle:

```cpp
auto popup = _new<AWindow>("Radius", 200_dp, 50_dp, dynamic_cast<AWindow*>(ASurface::current()), WindowStyle::MODAL);
popup->setContents(Vertical {
    Label { "Adjust radius." },
    Slider {
        .value = AUI_REACT(circle->radius / MAX_RADIUS),
        .onValueChanged = [circle](aui::float_within_0_1 s) { circle->radius = s * MAX_RADIUS; },
    },
});
connect(popup->closed, [] { /* commit the changes */ });
popup->show();
```

`closed` is emitted when the window is closed; close it from code with `close()`. Note that `MODAL` only affects the
look of the window. If the user should not interact with the parent meanwhile, call `blockUserInput()` on the parent
and `blockUserInput(false)` before the dialog is destroyed (see [ASurface]).

## Context menus

Override `composeContextMenu()` in a view. It returns an `AMenuModel`, which is a vector of `AMenuItem`s. It's called
every time the user right clicks (or long presses) the view, so the content can depend on the current state.
Returning an empty vector shows no menu.

```cpp
AMenuModel composeContextMenu() override {
    auto circle = *mHoveredCircle;
    if (circle == nullptr) {
        return {};
    }
    return {
        {
            .name = "Adjust radius...",
            .onAction = [this, circle] { showRadiusDialog(circle); },
        },
        { .type = AMenu::SEPARATOR },
        {
            .name = "Delete",
            .shortcut = AInput::DEL,
            .onAction = [this, circle] { remove(circle); },
            .enabled = canDelete(),
        },
    };
}
```

Items can be nested with `.type = AMenu::SUBLIST` and `.subItems`. Text views such as [ATextField] already provide
cut/copy/paste/select all this way.

To open a menu without a right click, call `AMenu::show(model)`.

## Popups attached to a view

Dropdown lists and menus use overlapping surfaces: lightweight windows drawn above the window contents.
`ASurface::createOverlappingSurface(position, size)` creates one; fill it with views and call `pack()`.
`view->getPositionInWindow()` helps to place it below a view. See the doc comment of `createOverlappingSurface` in
[ASurface] for an example.
