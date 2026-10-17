# Building a form

A form is a bunch of input views whose values are kept in your data. This page goes through the input views and ways to
connect them to your state. For the layout side, see [layout-managers].

## Input views at a glance

| Need                          | View                                |
|-------------------------------|-------------------------------------|
| one line of text              | [ATextField]                        |
| multiple lines of text        | [ATextArea]                         |
| integer in a range            | [ANumberPicker]                     |
| flag                          | [ACheckBox]                         |
| one of a few options          | [ARadioButton], [ADropdownList]     |
| value between 0 and 1         | [Slider]                            |
| action                        | [AButton]                           |

## State and layout first

Keep the values of a form in a state object made of properties, and describe the form with `Vertical` / `Horizontal`
containers. Anything that depends on the state is written as `AUI_REACT(...)`: AUI tracks the properties read inside the
expression and updates the view when they change. No manual refresh code is needed.

```cpp
struct Booking {
    AProperty<AString> name;
    AProperty<int> passengers = 1;
    AProperty<bool> isReturnFlight = false;

    bool isValid() const { return !name->empty() && *passengers > 0; }
};

...
setContents(Vertical {
    Horizontal {
        Label { "Name:" },
        _new<ATextField>() && state->name,
    },
    Label { AUI_REACT("Hello, {}!"_format(state->name)) },
    Label { AUI_REACT("Passengers: {}"_format(state->passengers)) },
    Button {
        .content = Label { AUI_REACT(state->isValid() ? "Book" : "Fill in the form") },
        .onClick = [=] { book(*state); },
    } AUI_LET { AObject::connect(AUI_REACT(state->isValid()), AUI_SLOT(it)::setEnabled); },
} AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } });
```

`Label { AUI_REACT(...) }` and `Button { .content = Label { AUI_REACT(...) } }` are the main tools: the form reads as a
description of what is shown for the given state, and handlers (`.onClick`) only change the state.

## Text input and two-way binding

The `text()` property of [ATextField] and [ATextArea] can be bound to an `AProperty<AString>` of your state with `&&`.
Typing updates the property; assigning the property updates the field.

<!-- aui:snippet examples/7guis/crud/src/main.cpp crud_form -->

`_form` lays out `{ label, view }` pairs in two columns. The same state can feed several views, because properties
notify all of their observers. Here, `mEditedUser.name` is bound to the text field; when the user presses "Create",
the code simply reads (and resets) it, and the text field goes empty by itself.

## Enabling a button when the form is valid

Express validity as a reactive expression over the state and connect it to `setEnabled` of the button:

```cpp
Button {
    .content = Label { "Book" },
    .onClick = [=] { /* ... */ },
} AUI_LET { AObject::connect(AUI_REACT(state->isValid()), AUI_SLOT(it)::setEnabled); },
```

This is how the 7GUIs "Flight Booker" does it. If the validity is used in several places, give it a name with
`APropertyPrecomputed`, which recomputes whenever a property it read has changed, as the 7GUIs "CRUD"
(`examples/7guis/crud`) does:

```cpp
APropertyPrecomputed<bool> mCreateEnabled = [this] { return !(mEditedUser.surname->empty() || mEditedUser.name->empty()); };
...
Button { Label { "Create" }, { me::createClicked } } & mCreateEnabled > &AView::setEnabled,
```

For details on properties and reactive expressions, see [property-system].

## Validating text

The field accepts whatever is typed. To validate, observe the text, parse it, and change the style of the field to
signal an error. The example parses the text of a field as a date and makes the background red if it is not one.

<!-- aui:snippet examples/7guis/flight_booker/src/main.cpp date_field -->

If you want to forbid some characters altogether, subclass [ATextField] and override `isValidText`
(see `ANumberPicker`'s inner field for an example).

## Passwords, read-only fields, limits

```cpp
auto password = _new<ATextField>();
password->setPasswordMode(true);          // hides symbols, disables copy
password->setMaxTextLength(64);

auto readOnly = _new<ATextField>();
readOnly->setEditable(false);
```

## Choosing from a list

[ADropdownList] takes a list model. Its `selectionId()` is a property, so it binds like anything else:

<!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp dropdown -->

The flight booker connects it to a boolean this way:

```cpp
AObject::connect(AUI_REACT(state->isReturnFlight ? 1 : 0), it->selectionId());
AObject::connect(it->selectionId().changed, AObject::GENERIC_OBSERVER, [=](int newSelection) {
    state->isReturnFlight = newSelection == 1;
});
```

## Numbers

[ANumberPicker] is an integer text field with plus/minus buttons. Set the range with `setMin` / `setMax`, read the value
with `getValue()` or observe `valueChanged` (final) and `valueChanging` (on every keystroke):

```cpp
auto picker = _new<ANumberPicker>();
picker->setMin(1);
picker->setMax(12);
picker->setValue(4);
AObject::connect(picker->valueChanged, AObject::GENERIC_OBSERVER, [](int64_t x) { /* ... */ });
```

[ADoubleNumberPicker] is the same for floating point numbers.

## Slider and progress

[Slider] works with a value from 0 to 1. It does not keep its own state: it displays `.value` and notifies you via
`.onValueChanged`; you update the state, and the value flows back.

<!-- aui:snippet examples/7guis/timer/src/main.cpp timer_slider -->

Multiply by your range to get a real value, as the timer does with its duration. A [AProgressBar] or `CircleProgressBar`
showing the same property displays progress; see its page for styling.

## Grouping

Wrap related inputs with [AGroupBox] (a frame with a title) and arrange them with `Vertical`, `Horizontal` or `_form`.
