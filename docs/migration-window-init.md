# Migration: WindowStyle to AWindow::Init

`WindowStyle`, `ACustomWindow`, `ACustomCaptionWindow` and `AWindow::setWindowStyle` are removed without a
deprecation period. This page maps the old API to the new one. See [AWindow] for the overview of the new API.

## Summary

| Old | New |
|---|---|
| `AWindow(name, w, h, parent, ws)` | `AWindow(AWindow::Init { ... })`; `AWindow(title, w, h)` still works |
| `WindowStyle::NO_MINIMIZE_MAXIMIZE` | `.decoration = DecorationDefault { .buttons = { .minimize = false, .maximize = false } }` |
| `WindowStyle::NO_RESIZE` | `.resizable = false` |
| `WindowStyle::MODAL` | `.parent = Parent { .window = *w, .modal = true }`, `.resizable = false`, buttons off if needed |
| `WindowStyle::NO_DECORATORS` | `.decoration = DecorationBorderless {}` |
| `WindowStyle::NO_TITLEBAR` | `.decoration = DecorationExtended {}` + `Titlebar` |
| `WindowStyle::SYS` | removed from the public API; popups are created by AUI only (`AWindow::isPopup`) |
| `ACustomWindow` | `DecorationBorderless` + `Titlebar` |
| `ACustomCaptionWindow` | `DecorationExtended` + `Titlebar` (+ `Stacked`) |
| `getCaptionContainer()`, `getContentContainer()` | your own layout |
| `setWindowStyle(...)` at runtime | `Init` at creation, `setResizable` at runtime |
| `getWindowTitle()` | `title()` property (`setTitle()` and `titleChanged` are new) |
| `quit()` | `close()` |
| `.window-title` ASS class | `t<ATitlebar>()` |
| `dragBegin` / `dragEnd` signals | removed |
| `AWindow::wrapViewToWindow(view, title, w, h, parent, ws)` | `wrapViewToWindow(view, AWindow::Init { ... })` |

## MODAL is no longer implicit

`WindowStyle::MODAL` was `NO_MINIMIZE_MAXIMIZE | NO_RESIZE`, and had no bit of its own. As a result, a window created
with only `NO_RESIZE` and a parent was **silently modal** on Windows (the parent got disabled). Now modality is
explicit: `NO_RESIZE` becomes `.resizable = false`, and the window is modal only if `parent->modal` is set.

Before:
```cpp
AWindow("New game", 100, 100, minesweeper, WindowStyle::MODAL);
```
After:
```cpp
AWindow(AWindow::Init {
    .title = "New game",
    .width = 100_px,
    .height = 100_px,
    .parent = AWindow::Init::Parent { .window = *minesweeper, .modal = true },
    .decoration = AWindow::Init::DecorationDefault { .buttons = { .minimize = false, .maximize = false } },
    .resizable = false,
});
```

Before (not modal on Linux, modal on Windows):
```cpp
AWindow("Jump to coords", 854_dp, 500_dp, parent, WindowStyle::NO_RESIZE);
```
After:
```cpp
AWindow(AWindow::Init {
    .title = "Jump to coords",
    .parent = AWindow::Init::Parent { .window = *parent },
    .resizable = false,
});
```

## setWindowStyle at runtime

Most of the style was applied only at window creation on some platforms (`NO_TITLEBAR` on GTK, override-redirect on X11,
caption buttons on Windows), so `setWindowStyle` looked like a runtime API but was not. Pass everything via `Init`;
`AWindow::setResizable` is the only style which can be changed at runtime.

Before:
```cpp
setWindowStyle(WindowStyle::MODAL);
```
After: pass `Init` to the `AWindow` constructor (see above).

## ACustomWindow

Before:
```cpp
class MySplash : public ACustomWindow {
public:
    MySplash() : ACustomWindow("Splash", 400_dp, 300_dp) {
        setContents(Centered { Label { "Loading..." } });
    }
};
```
After:
```cpp
class MySplash : public AWindow {
public:
    MySplash() : AWindow(Init { .title = "Splash", .width = 400_dp, .height = 300_dp,
                                .decoration = Init::DecorationBorderless {} }) {
        setContents(Vertical {
            Titlebar {},   // makes the window draggable and shows the window title
            Centered { Label { "Loading..." } } AUI_LET { it->setExpanding(); },
        });
    }
};
```

## ACustomCaptionWindow

Before:
```cpp
auto w = _new<ACustomCaptionWindow>("Title", 400_dp, 300_dp, true /* stacked */);
w->getCaptionContainer()->addView(Label { "My caption" });
w->getContentContainer()->addView(content);
```
After:
```cpp
auto w = _new<AWindow>(AWindow::Init {
    .title = "Title",
    .width = 400_dp,
    .height = 300_dp,
    .decoration = AWindow::Init::DecorationExtended {},
});
w->setContents(Stacked {
    content,
    Vertical {
        Titlebar { Label { "My caption" } },
        SpacerExpanding {},
    },
});
```
For the non-stacked variant (contents below the caption) use `Vertical { Titlebar { ... }, content }`.

A `DecorationExtended` window has `.window-extended` ASS class, which removes the default `8_dp` padding of
[AWindow] (`ACustomCaptionWindow` did the same), so the contents reach the window edges.

The titlebar does not have the hardcoded `30px` height anymore: its height is the height of its contents (at least
the height of the window controls).

## Window title in a custom titlebar

`ACustomCaptionWindow` drew the window title with an AUI label. Now `Titlebar {}` without contents contains
`WindowTitle {}`, which marks the place of the title: on macOS and GTK the OS draws the native title there, on Windows
and X11 AUI draws a label (left-aligned on Windows, centered on Linux). Put `WindowTitle {}` explicitly to combine it
with other views. Either way it shows `AWindow::title`, so `setTitle()` updates both the OS-visible title and the one
in the window.

Before:
```cpp
w->getCaptionContainer()->addView(Label { "My app" });   // duplicates the window title
```
After:
```cpp
Titlebar { Horizontal { Icon { ":img/logo.svg" }, WindowTitle {} } }
```
If a label differs from the title, bind it: `Label { .text = AUI_REACT("{} - Editor"_format(*w->title())) }`.

## close()

`close()` is now the single way to close a window and it is terminal on all platforms; `quit()` is removed.
Before, Windows and macOS destroyed the window (but a later `show()` produced an invisible window keeping the
application alive), X11 only unmapped it, and GTK silently did nothing (and emitted `closed` twice).

Now: `closed` is emitted exactly once, the modal parent is unblocked, the native window is destroyed and the window is
removed from `AWindowManager`. Calling `close()` twice is a no-op. `show()` after `close()` logs an error and does
nothing (an assertion in debug builds); create another window instead. Use `hide()` / `show()` for a window which is
supposed to reappear.
