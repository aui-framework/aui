# Window decorations, modality and titlebar

`AWindow` is created with `AWindow::Init` — an aggregate describing everything that can't be changed afterwards:

```cpp
_new<AWindow>(AWindow::Init {
    .title = "Settings",
    .width = 400_dp,
    .height = 300_dp,
    .parent = AWindow::Init::Parent { .window = *this, .modal = true },
    .decoration = AWindow::Init::DecorationDefault { .buttons = { .minimize = false, .maximize = false } },
    .resizable = false,
});
```

Meaningless combinations do not compile: modality exists only as a part of `parent`, window buttons exist only in
decorations which have them.

## Decorations

| Decoration | What it is | Replaces |
|---|---|---|
| `DecorationDefault { buttons }` | Ordinary OS-drawn titlebar and border. | `WindowStyle::DEFAULT`, `NO_MINIMIZE_MAXIMIZE` |
| `DecorationExtended { buttons }` | Window contents extend into the titlebar. Native window controls are kept where the platform has them. | `ACustomCaptionWindow` |
| `DecorationBorderless { shadow }` | No titlebar, no buttons. Native border, shadow and rounded corners are kept where possible. | `ACustomWindow`, `NO_DECORATORS` |

`AWindow::Init::Buttons` toggles `minimize`, `maximize` and `close`.

## ATitlebar

With `DecorationExtended` (and `DecorationBorderless`, to make the window draggable) you fill the titlebar yourself
using `ATitlebar`:

```cpp
setContents(Stacked {
    content,
    Vertical {
        _new<ATitlebar>(Horizontal { Label { "My app" } }),
        SpacerExpanding {},
    },
});
```

`ATitlebar` lays out as `Horizontal { AWindowControls(START), contents, AWindowControls(END) }`. Its height is the
maximum of the contents' height and the window controls' height: to make the titlebar taller, put a taller view in it.

`ATitlebar` is always a window drag handle: dragging any point under it moves the window and double click
maximizes/restores it. Buttons and views with `.override-title-dragging` ASS class are not draggable.

`AWindow::windowControlsInsets` is a property reporting the space occupied by native window controls (traffic lights on
macOS, `GtkWindowControls` on GTK); `AWindowControls` uses it to keep the contents out of their way.

## Modality

A modal window blocks user input of its parent until closed. Having `parent` without `modal` produces an ordinary
child window — it does not block anything, even when it is not resizable.

## Lifecycle

Only title and resizability can change at runtime:

| What | API |
|---|---|
| title | `AWindow::setTitle` / `AWindow::title` |
| resizability | `AWindow::setResizable` / `AWindow::isResizable` |
| visibility | `AWindow::show` / `AWindow::hide`, reversible |
| closing | `AWindow::close`, **terminal** |

`close()` works the same on all platforms: `closed` is emitted exactly once, the modal parent is unblocked, the native
window is destroyed. A closed window can't be shown again — create another one. Calling `show()` on a closed window logs
an error and does nothing. Closing by the close button calls `AWindow::onCloseButtonClicked`, which calls `close()`;
override it to refuse closing.

## Platform specifics

| | Windows | macOS | Linux GTK/adw | Linux X11 | Android, iOS, web |
|---|---|---|---|---|---|
| `Default` buttons | window style bits; disabled close is greyed | style mask | `decoration-layout` of the header bar | Motif hints (window manager may ignore) | ignored |
| `Extended` window controls | drawn by AUI, hit-tested natively (snap layouts) | native traffic lights, aligned to `ATitlebar` | native `GtkWindowControls` | drawn by AUI | none |
| `Borderless` shadow | DWM frame | system shadow | CSD shadow | **not available**, `shadow` is ignored | ignored |
| `parent` | owner window | child window | transient | transient | ignored |
| `modal` | parent is disabled | parent ignores input | `gtk_window_set_modal` | `_NET_WM_STATE_MODAL`, parent ignores input | ignored |

Known gaps (not related to decorations): minimize/maximize are stubs on macOS, there is no `minimized` signal on all
platforms, `moveToCenter` is empty on Linux X11.
