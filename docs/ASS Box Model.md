# AUI Box Model

The AUI Box Model is layout structure for all types of AViews, which combines margin, padding, border, and background
elements to create a visual container, defining the outer appearance of each AView as a rectangular block on-screen.

## Box key components

- **View's content box**: the area are whose size is defined by @ref AView::getContentMinimumSize() and
  @ref AView::getContentSize(). The content area is the innermost area, wrapping view's specific contents only (i.e,
  label's text); control it by implementing @ref AView::getContentMinimumWidth() and
  @ref AView::getContentMinimumHeight().
- **View's box**: the area whose size is defined by AView::size(). Use @ref ass::FixedSize, @ref ass::MinSize and
  @ref ass::MaxSize to control it.
  
  Views are responsible to handle its padding properly. As an AUI user, you wouldn't need to bother about that unless
  you are implementing custom @ref AView::render().

  @ref ass::BackgroundSolid "BackgroundSolid" and similar @ref ass properties are fit into this box.
  
  @ref ass::Border "Border" property outlines this box from the inner side, and never affects the layout; it's just a
  visual trait.
- **View's margin**: the margin is the outermost layer, wrapping the content, padding, and border as whitespace between
  this box and other elements; control it using @ref ass::Margin.
  
  Layout manager of view's parent is responsible to handling margin properly. All @ref layout_managers "layout managers"
  tend to honor children margins; with an exception to @ref AWindow (it's margin has no effect) and @ref AAbsoluteLayout
  (positioning and sizing is defined manually). As an AUI user, you wouldn't need to bother about that unless you are
  implementing custom layout manager.

# Comparison to CSS

AUI shares principles with [web technologies](https://developer.mozilla.org/en-US/docs/Learn_web_development/Core/Styling_basics/Box_model#what_is_the_css_box_model)
with some exceptions. In AUI:

- border never affects layout
- element's size includes padding, thus it differs from "content area"

AUI's box model is equal to CSS's ["alternate box model"](https://developer.mozilla.org/en-US/docs/Learn_web_development/Core/Styling_basics/Box_model#playing_with_box_models),
with an exception to border.
