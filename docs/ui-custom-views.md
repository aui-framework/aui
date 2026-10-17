# Creating a custom view and custom painting

When no standard view does what you need, derive from [AView]. A custom view is responsible for three things: telling
its size, painting itself and reacting to input. Everything else (styling, focus, hover, data binding) it inherits.

## Painting

Override `render(ARenderContext)`. Call the base implementation first so the view's background and border from the
stylesheet are painted below your content, then draw with `ctx.render`, which is an [IRenderer]. Coordinates are in
pixels relative to the top left corner of the view.

<!-- aui:snippet examples/7guis/circle_drawer/src/main.cpp custom_render -->

The view above (from the 7GUIs Circle Drawer) also sets its look up in the constructor with `setCustomStyle`, so it
needs no stylesheet. Whatever [IRenderer] has to offer can be used: rectangles, rounded rectangles, borders, lines,
strings, drawables. Brushes (`ASolidBrush`, gradients...) define how the shape is filled.

When data your view displays changes, call `redraw()`. The example does this by connecting signals of its state:

```cpp
connect(mState->circles.changed, me::redraw);
connect(mHoveredCircle.changed, me::redraw);
```

!!! warning

    `render` may be called many times per second or, on the contrary, be skipped when nothing changes; moreover AUI may
    cache the view's picture (see [render-to-texture]). Do not mutate application state in `render`; only paint.

## Sizing

By default, a custom view has a minimal size which can be changed from the stylesheet (`MinSize`, `FixedSize`,
`Expanding`, `MaxSize`; see [ass_properties]). Setting `Expanding()` makes the view fill the available space, which is
what the circle drawer does.

If the preferred size depends on the contents, override `onIntrinsicMeasure`. It receives constraints and returns the
size of the content area in pixels. Measurement must not change the layout; if something that affects the size has
changed, call `requestLayout()` and the view will be measured again.

```cpp
glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
    return { 100, int(mValues.size()) * 20 };
}
```

The layout system is described in [layout-managers]. If the view contains child views, derive from
`AViewContainerBase` and set a layout instead.

## Reacting to input

Override the pointer and keyboard functions; see [ui-input-events] for the full list. The circle drawer finds the circle
under the pointer in `onPointerMove` and adds a new one in `onPointerReleased`. Always call the base implementation,
otherwise `clicked` and hover state stop working.

Set the cursor with `setCursor(ACursor::POINTER)`, and offer a right click menu by overriding `composeContextMenu()`,
see [ui-dialogs-and-menus].

## Exposing state

Views in AUI are expected to cooperate with the property system and be usable from the declarative DSL, like
`Label { AUI_REACT(...) }` or `Slider { .value = ..., .onValueChanged = ... }`. The idiomatic way to achieve this is to
put the view behind a struct in `namespace declarative`: its fields are the inputs (`contract::In` for values flowing
in, a callback or `contract::Slot` for events flowing out), and `operator()` builds the view out of existing ones.

```cpp
namespace declarative {
struct Chip {
    contract::In<AString> text;
    contract::Slot<> onClick;

    _<AView> operator()() {
        return Button {
            .content = Label { .text = std::move(text) },
            .onClick = std::move(onClick),
        }();
    }
};
}   // namespace declarative
```

Usage is the same as with the built-in views:

```cpp
Chip {
    .text = AUI_REACT("Clicked {} times"_format(state->count)),
    .onClick = [=] { ++state->count; },
}
```

The struct does not own any state; it displays what it is given and reports what the user did.

See `declarative::Slider` ([ASlider.h](https://github.com/aui-framework/aui/blob/master/aui.views/src/AUI/View/ASlider.h))
for a complete reference implementation, and [property-system] for details on reactive expressions.

## Drawing with OpenGL

A view can draw with raw OpenGL, too. In `render`, restore the GL state AUI relies on after your calls. The
`examples/ui/opengl_simple` example draws a triangle this way, and `examples/app/fractal` renders a fractal with a
shader in a custom view.

## Images

To show a picture, you don't need a custom view: `Icon { ":img/logo.svg" }` (backed by [ADrawableView]) shows an SVG or
raster image from the assets (see [aui-assets]), and [ass] has `BackgroundImage`.
