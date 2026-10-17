# Custom window titlebar blur

<!-- aui:example ui -->
Window with extended decoration, whose titlebar blurs the contents scrolled under it.

This example demonstrates [AWindow::Init::DecorationExtended] with [declarative::Titlebar] overlaid on top of an
[AScrollArea] using [AStackedLayout].

## How it works

The blur is achieved by layering:

1. [AScrollArea] fills the whole window, including the area behind the titlebar.
2. [declarative::Titlebar] is stacked on top of it as an overlay. Its style sets a translucent background
   (`BackgroundSolid` with a half-transparent white) and [ass::Backdrop] with `Backdrop::GaussianBlur`, which blurs
   whatever is rendered underneath the titlebar.
3. [AScrollArea] has [AOverflow] set to `AOverflow::VISIBLE`. By default, the scroll area clips its contents to its
   bounds, so overscrolled content (rubber-banding past the edges) would be cut off. With `AOverflow::VISIBLE`, the
   content is not clipped and can slide under the titlebar overlay; as a consequence, it ends up in the backdrop and
   gets blurred.

Without step 3 the titlebar would blur only the content that lies within the scroll area, and overscrolled content
would disappear instead of being blurred.
