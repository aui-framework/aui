# Custom View: Struct or Function

<!-- aui:example ui -->
Two ways to define a reusable custom view in AUI: a plain **function** and a **callable struct** with named
(designated-initializer) parameters.

Both build the same `sliderMinMax` component — a `Slider` remapped from an arbitrary `[min, max]` range onto the
underlying `[0, 1]` slider value. The example wires them to a shared, reactive `State::degress` property so you can see
that both stay in sync while dragging either slider.

## Two styles

### Function

```cpp
_<AView> sliderMinMax(contract::In<float> value,
                      std::function<void(float)> onValueChanged,
                      contract::In<float> min = 0.f,
                      contract::In<float> max = 100.f);
```

- Positional call site: `sliderMinMax(value, cb, 0.f, 360.f)`.
- Reactivity "just works": the parameters are local variables, so `AUI_REACT([=]...)` copies each `contract::In`
  (bumping its `shared_ptr` refcount) and keeps the reactive chain alive after the function returns.

### Struct

```cpp
struct SliderMinMax {
    contract::In<float> value;
    std::function<void(float)> onValueChanged;
    contract::In<float> min = 0.f;
    contract::In<float> max = 100.f;

    _<AView> operator()();
};
```

- Named / order-independent call site via designated initializers — self-documenting and consistent with the rest of
  the declarative DSL (`Slider { ... }`, `Label { ... }`).
- Easy to extend with new optional fields without breaking existing usages.

## The `this`-capture trap (and why it fails to build)

`AUI_REACT(expr)` expands to a `[=]` lambda. `[=]` copies **local variables** by value, but inside a member function it
implicitly captures **`this`** and reaches data members through it. Since a custom-view struct is typically a
temporary, that `this` dangles at the end of the enclosing full-expression while the precomputed reactive property
outlives it — silently breaking reactivity.

To make this impossible, `AUI_REACT` promotes the C++20 "implicit `this` capture via `[=]`" deprecation into a hard
**compile error**. So this does **not** build:

```cpp
_<AView> operator()() {
    return Slider {
        .value = AUI_REACT((*value - *min) / (*max - *min)), // ❌ captures `this` → compile error
        ...
    };
}
```

The fix is to copy the members into locals first, so `AUI_REACT` captures the copies (each a `contract::In` holding a
`shared_ptr` to the reactive expression) instead of `this`:

```cpp
_<AView> operator()() {
    auto value = std::move(this->value);
    auto min = std::move(this->min);
    auto max = std::move(this->max);
    return Slider {
        .value = AUI_REACT((*value - *min) / (*max - *min)), // ✅ captures local copies
        ...
    };
}
```

## Source Code

<!-- aui:include examples/ui/ui_struct_or_func/src/main.cpp -->
