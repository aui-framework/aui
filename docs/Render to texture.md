Starting from 6.x.x, AUI supports render to texture optimization (experimental). That is, instead of redrawing
everything each frame (knows as immediate mode) framework redraws only views that actually updated, copying persisting
pixel data from the last frame.

This may gradually improve performance especially if there are a lot of static views.

Starting from 6.x.x devtools has several options to debug and visualise render-to-texture optimization.

![Devtools](https://github.com/aui-framework/aui/blob/master/docs/imgs/Screenshot_20241014_054912?raw=true)

## Highlight redraw requests

When enabled, highlights the views that requested repaint with purple overlay.

This setting shows which pixel areas should be repainted in the best case. Also you can find views that trigger window
repaint event.

![Example window](https://github.com/aui-framework/aui/blob/master/docs/imgs/Screenshot_20241014_054912?raw=true)

In this example, text field and spinner box are highlighted. Text field has to display cursor blinking, spinner box
spins infinitely so it always highlighted.

## Render to texture decay

When enabled, gradually grays out pixel areas that didn't repaint. That is, redrawn views would appear bright and
saturated.