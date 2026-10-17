# Animating views

AUI has two ways to make things move: animators attached to a view, and animating a property yourself.

## Animators

An animator ([AAnimator]) transforms how a view is painted over a period of time. Create one, set the duration in
seconds and assign it to the view. The view is repainted continuously until the animation finishes.

```cpp
auto animator = _new<ATranslationAnimator>(glm::vec2 { -100, 0 });   // slide in from the left
animator->setDuration(0.3f);
view->setAnimator(animator);
```

The other stock animators are `ARotationAnimator` (between two [AAngleRadians], i.e., `0_deg` and `90_deg`) and
`ASizeAnimator`. `AAnimator::combine({ a, b })` runs several at once; `setRepeating(true)` makes the animation loop
(useful for a busy indicator like [ASpinner]); `setCurve` replaces the easing. Ready-made curves live in
`aui::animation_curves` (`Linear`, `CubicBezier`, `Standard`).

Animators change only the painting: the layout and hit testing of the view are not affected, except for
`ASizeAnimator`, which really changes the size.

A custom animator is a class overriding `doAnimation(view, theta, renderer)` where `theta` goes from 0 to 1.

## Animating a property

Anything that is a function of a property can be animated by changing that property over time. Drive it with an
[ATimer], and let reactive expressions or signals do the rest. The timer example (7GUIs) updates the current time ten
times a second, and a progress bar follows:

<!-- aui:snippet examples/7guis/timer/src/main.cpp example -->

For indeterminate waiting, [ASpinner] and `ASpinnerV2` rotate by themselves; use [AProgressBar] when you know the
progress.

## Performance

Every frame of an animation is a repaint. Keep animated areas small and do not animate in a view that is expensive to
paint; consider [render-to-texture] to cache a complex subtree.
