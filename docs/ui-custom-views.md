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

Views in AUI are expected to cooperate with the property system. Instead of getter/setter pairs only, expose a property
and a signal, so that the view can be used in data bindings and reactive expressions:

```cpp
class RatingView : public AView {
public:
    auto rating() const { return APropertyDef { this, &RatingView::getRating, &RatingView::setRating, ratingChanged }; }

    int getRating() const { return mRating; }
    void setRating(int rating) {
        if (mRating == rating) {
            return;
        }
        mRating = rating;
        redraw();
        emit ratingChanged(rating);
    }

signals:
    emits<int> ratingChanged;

private:
    int mRating = 0;
};
```

Now `RatingView` can be used like `_new<RatingView>() && state->rating`; see [property-system].

## Drawing with OpenGL

A view can draw with raw OpenGL, too. In `render`, restore the GL state AUI relies on after your calls. The
`examples/ui/opengl_simple` example draws a triangle this way, and `examples/app/fractal` renders a fractal with a
shader in a custom view.

## Images

To show a picture, you don't need a custom view: `Icon { ":img/logo.svg" }` (backed by [ADrawableView]) shows an SVG or
raster image from the assets (see [aui-assets]), and [ass] has `BackgroundImage`.
