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

## Text input and two-way binding

The `text()` property of [ATextField] and [ATextArea] can be bound to an `AProperty<AString>` of your state with `&&`.
Typing updates the property; assigning the property updates the field.

<!-- aui:snippet examples/7guis/crud/src/main.cpp crud_form -->

`_form` lays out `{ label, view }` pairs in two columns. The same state can feed several views, because properties
notify all of their observers. Here, `mEditedUser.name` is bound to the text field; when the user presses "Create",
the code simply reads (and resets) it, and the text field goes empty by itself.

## Enabling a button when the form is valid

Derive validity from the state with `APropertyPrecomputed`, which recomputes whenever a property it read has changed,
and connect it to the `setEnabled` of the button:

```cpp
APropertyPrecomputed<bool> mCreateEnabled = [this] { return !(mEditedUser.surname->empty() || mEditedUser.name->empty()); };
...
Button { Label { "Create" }, { me::createClicked } } & mCreateEnabled > &AView::setEnabled,
```

The 7GUIs "CRUD" example in `examples/7guis/crud` has the entire code. The "Flight Booker" does the same inline, with
`AUI_REACT`:

```cpp
Button {
    .content = Label { "Book" },
    .onClick = [=] { /* ... */ },
} AUI_LET { AObject::connect(AUI_REACT(state->isValid()), AUI_SLOT(it)::setEnabled); },
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
