# AUI Contacts {#example_contacts}

@auiexample{ui}
Usage of @ref AUI_DECLARATIVE_FOR to make a contacts-like application.

@image html docs/imgs/Screenshot_20250328_153702.png

UI is defined using a declarative syntax, where the structure and layout of the UI are specified as a series of function
calls.

The application uses an @ref AProperty named `mContacts` to store a vector of contact objects. Each contact object has
properties like `displayName`, `note`, etc.
