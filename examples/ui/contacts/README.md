# AUI Contacts

<!-- aui:example ui -->
Usage of [AUI_DECLARATIVE_FOR] to make a contacts-like application.

![](imgs/Screenshot_20250328_153702.png)

UI is defined using a declarative syntax, where the structure and layout of the UI are specified as a series of function
calls.

The application uses an [AProperty] named `mContacts` to store a vector of contact objects. Each contact object has
properties like `displayName`, `note`, etc.
