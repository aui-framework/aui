Devtools is a UI set of tools integrated in `aui.views` and thus in every AUI application. The devtools window can be
invoked by hitting `CTRL+F12` on AUI-based application.

@image html Screenshot_20241212_064123.png

# Layout tab

The most notable functionality of devtools is the `Layout` tab. Here you can discover your view hierarchy and check
which @ref ass "ASS" rules were applied to your view.

You can hold `CTRL` and hover over your application to peek views:

@image html Screenshot_20241212_064400.png


# Disabling devtools in production

By default, devtools window is accessible in both debug and release builds of AUI applications so in theory it can be
accessed by users of your application. If it renders unacceptable for you, you can override and stub
AWindowBase::createDevtoolsWindow method in your AWindow.
