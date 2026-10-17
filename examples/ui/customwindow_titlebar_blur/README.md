# Custom window titlebar blur

<!-- aui:example ui -->
Window with extended decoration, whose titlebar blurs the contents scrolled under it.

![](imgs/Screenshot_20261017_082728.png)

This example demonstrates [AWindow::Init::DecorationExtended] with [declarative::Titlebar] overlaid on top of an
[AScrollArea] using [AStackedLayout].

## How it works

The window contents are a [declarative::Stacked] of two layers:

1. Content layer: a [declarative::Vertical] with an [ASpacerFixed] on top and the [AScrollArea] below it. The spacer's
   height is bound to the titlebar's measured height (`AUI_REACT(AMetric(titlebar->size()->y, AMetric::T_PX))`), so
   the initial position of the contents is shifted from beneath the titlebar, while scrolled content can still go under
   it.
2. Titlebar layer: a [declarative::Vertical] with [declarative::Titlebar] at the top and a `SpacerExpanding` below, so
   the titlebar does not capture the rest of the window. Its style sets a translucent background (`BackgroundSolid`
   with a half-transparent white) and [ass::Backdrop] with `Backdrop::GaussianBlur`, which blurs whatever is rendered
   underneath the titlebar.

By default, the scroll area clips its contents to the bounds of [AScrollAreaViewport]. Since the scroll area no longer
extends under the titlebar, its viewport is switched to `AOverflow::VISIBLE` with an extra stylesheet (the view is
marked with the `.scrollarea-viewport-hack` class and the rule targets `t<AScrollAreaViewport>()`). The content is then
not clipped and can slide under the titlebar overlay; as a consequence, it ends up in the backdrop and gets blurred.

Without this, the titlebar would blur only the content that lies within the scroll area, and scrolled content would
disappear at the edge of the viewport instead of being blurred.
