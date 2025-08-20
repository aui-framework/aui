# 7GUIs CRUD

<!-- aui:example 7guis -->
Create/Read/Update/Delete example.

![](imgs/Screenshot_20250401_074431.png)

Challenges: separating the domain and presentation logic, managing mutation, building a non-trivial layout.

The task is to build a frame containing the following elements: a textfield Tprefix, a pair of textfields Tname and
Tsurname, a listbox L, buttons BC, BU and BD and the three labels as seen in the screenshot. L presents a view of the
data in the database that consists of a list of names. At most one entry can be selected in L at a time. By entering a
string into Tprefix the user can filter the names whose surname start with the entered prefix—this should happen
immediately without having to submit the prefix with enter. Clicking BC will append the resulting name from
concatenating the strings in Tname and Tsurname to L. BU and BD are enabled iff an entry in L is selected. In contrast
to BC, BU will not append the resulting name but instead replace the selected entry with the new name. BD will remove
the selected entry. The layout is to be done like suggested in the screenshot. In particular, L must occupy all the
remaining space.

CRUD (Create, Read, Update and Delete) represents a typical graphical business application. The primary challenge is the
separation of domain and presentation logic in the source code that is more or less forced on the implementer due to the
ability to filter the view by a prefix. Traditionally, some form of MVC pattern is used to achieve the separation of
domain and presentation logic. Also, the approach to managing the mutation of the list of names is tested. A good
solution will have a good separation between the domain and presentation logic without much overhead (e.g. in the form
of toolkit specific concepts or language/paradigm concepts), a mutation management that is fast but not error-prone and
a natural representation of the layout (layout builders are allowed, of course, but would increase the overhead).
