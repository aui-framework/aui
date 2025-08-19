# Render to texture

Starting from 6.x.x, AUI supports render to texture optimization (experimental). That is, instead of redrawing
every view each frame (known as immediate mode) framework redraws views that actually updated, copying persisting
pixel data from the last frame. Sometimes this technique is referred as backbuffering.

This may gradually improve performance especially if there are a lot of static views.

Starting from 6.x.x, [devtools] has several options to debug and visualise render-to-texture optimization.

![](imgs/Screenshot_20241014_054912.png)

## Highlight redraw requests

When enabled, highlights the views that requested repaint with purple overlay.

This setting shows which pixel areas should be repainted. Also, you can find views that trigger window repaint event.

![](imgs/Screenshot_20241014_055322.png)

In this example, text field and spinner box are highlighted. Text field has to display cursor blinking, spinner box
spins infinitely so it always highlighted.

## Render to texture decay

When enabled, gradually grays out pixel areas that were copied from previous frame. That is, redrawn views would appear
bright and saturated and views that didn't requested repaint become grayed out.