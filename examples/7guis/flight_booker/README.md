# 7GUIs Flight Booker

<!-- aui:example 7guis -->
Flight Booker.

![](imgs/wjhsdflksdnjv.webp)

Challenge: Constraints.

The task is to build a frame containing a combobox C with the two options “one-way flight” and “return flight”, two
textfields T1 and T2 representing the start and return date, respectively, and a button B for submitting the selected
flight. T2 is enabled iff C’s value is “return flight”. When C has the value “return flight” and T2’s date is strictly
before T1’s then B is disabled. When a non-disabled textfield T has an ill-formatted date then T is colored red and B is
disabled. When clicking B a message is displayed informing the user of his selection (e.g. “You have booked a one-way
flight on 04.04.2014.”). Initially, C has the value “one-way flight” and T1 as well as T2 have the same (arbitrary)
date (it is implied that T2 is disabled).

The focus of Flight Booker lies on modelling constraints between widgets on the one hand and modelling constraints
within a widget on the other hand. Such constraints are very common in everyday interactions with GUI applications. A
good solution for Flight Booker will make the constraints clear, succinct and explicit in the source code and not hidden
behind a lot of scaffolding.

<!-- aui:include examples/7guis/flight_booker/src/main.cpp -->

## Regex Library

For validation in this example, we've chosen using regex technology, as it's fairly simple and extensible way to
make parsers.

Despite STL provides regex implementation, it varies from compiler to compiler, compiles the regex expression at runtime
only, and some platforms may even lack builtin regex library. To avoid possible issues, custom implementation should be
used.

Although AUI does not provide a regex parser on its own, nothing stops you from [AUI_BOOT_3RDPARTY] "using AUI.Boot"
in order to pull awesome 3rdparty implementation of your choice that suits your exact needs. For this example, we've
chosen [ctre](https://github.com/hanickadot/compile-time-regular-expressions), as it evaluates the regex expression at
compile-time, emitting effective code, as if we were validating the string manually.
