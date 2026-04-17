This folder contains the source XML for DBus methods.

Given that we need to use this interface in the dbus-cxx library,
we have a chicken-and-egg problem: how do you generate the
code to talk with the dbus-daemon before you build the library,
since to generate the code you need to build dbus-cxx-xml2cpp,
but to build that you need the library.

The solution here is to auto-generate this code and then check
it in.  If the XML changes and/or the data model changes,
you will need to re-run dbus-cxx-xml2cpp on this file, and commit
back the the changed DBusDaemonProxy.\* files

