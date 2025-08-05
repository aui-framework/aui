# UI Building Overview

AUI Framework provides a comprehensive set of tools and libraries for building robust, scalable, and maintainable user
interfaces. With its modular architecture and extensive feature set, developers can create complex UI components with
ease. See @ref "docs/Getting started with AUI.md" for quick start.

## Views

@ref useful_views "Views" are basic units to build UIs with AUI. There is a wide range of pre-built views for common UI
elements, such as buttons, labels, and text inputs.

![](imgs/Screenshot_20241218_144940.png)

## Layout

@ref layout_managers "Layout" is flexible and customizable way of managing your views in a logical and visually
appealing manner. From simple linear layouts to complex grid-based adaptive arrangements.

![](imgs/Screenshot_20210714_172900.png)

## Styles

@ref ass "ASS" supports for custom themes and styling using CSS-like syntax. Customize the look and feel of your UI
using robust styling and theming capabilities.

![](imgs/owrfuihw34iosdfjnfj.jpg)

## Event handling with signal-slot

@ref signal_slot can be used to handle UI events and respond to them accordingly.

```cpp
mOkButton = _new<AButton>("OK");
connect(mOkButton->clicked, [] { 
    ALogger::info("Example") << "The button was clicked";
});
```

## Data Binding

@ref property_system "Data binding", based on @ref signal_slot "signal-slot", allows to establish a connection between
UI and application data objects.

![](imgs/Screenshot_20250109_065134.png)

## Devtools

@ref docs/Devtools.md "AUI Devtools" provide a set of features that makes it easier to design, test, and debug your UI
components. With visual debugging tools, and more, you can streamline your development workflow and focus on building
amazing user experiences.

![](imgs/Screenshot_20241212_064400.png)

## API Reference

- @ref useful_views "All Views"
- @ref ass "ASS"
    - @ref ass_selectors "Selectors"
    - @ref ass_properties "Properties"
