# Lists, scrolling and tabs

This page is about views that hold and organize other content. To create views from a collection of your data
(the usual, reactive way), see [aforeachui]; the classic model-based views are described below.

## Scrolling

Wrap contents with [AScrollArea] to make them scrollable. The area expands by default, so give it room in the parent
layout (or limit it with `MaxSize` / `FixedSize`).

```cpp
AScrollArea::Builder().withContents(
    Vertical {
        Label { "Lots of..." },
        Label { "...content" },
    }
).build()
```

- `setStickToEnd(true)` keeps the area scrolled to the end as the content grows, which fits chats and logs;
- `scrollTo(view)` scrolls so that the given view is visible;

[ATextArea] does not scroll by itself, to avoid nested scrolling: put it into a scroll area, as the notes example does.

## List of views from data

[AForEachUI] and the `AUI_DECLARATIVE_FOR` macro create one view per item of an `AProperty<AVector<...>>` and keep them
in sync with it.. See [aforeachui].

## Model-based list

[AListView] displays a model of strings. A model is a class implementing `IListModel<AString>`, which only needs to tell
the number of items and to return an item by its index. When the data changes, the model emits `dataInserted`,
`dataRemoved` or `dataChanged` so the view updates.

<!-- aui:snippet examples/ui/views/src/DemoListModel.cpp demo_list_model -->

<!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp list_view -->

`AListModel<T>` is a ready-to-use model backed by a vector, and the one that [ADropdownList] takes:
`AListModel<AString>::make({ "one", "two" })`.

The selection is available as `getSelectionModel()` and the `selectionChanged` signal. `setAllowMultipleSelection(true)`
enables multiple selection; `itemDoubleClicked` tells when an item was double clicked.

Prefer [AForEachUI] when items need more than a string. `AListView` suits simple lists, especially with an existing
model.

## Trees

[ATreeView] shows an `ATreeModel<AString>`: an item has a name and optional children. Selection and hovering are
reported with `itemSelected`, `itemMouseClicked`, `itemMouseDoubleClicked` and `itemMouseHover`.

```cpp
_new<ATreeView>(_new<ATreeModel<AString>>(AVector<ATreeModel<AString>::Item>{
    { "dir1", { { "file1" }, { "file2" } } },
    { "dir2", { { "file3" } } },
}))
```

## Tabs

[ATabView] has a row of tab buttons and switches between added views:

```cpp
auto tabs = _new<ATabView>();
tabs->addTab(Centered { Label { "General settings" } }, "General");
tabs->addTab(Centered { Label { "About the app" } }, "About");
```

If tabs should be switched by something else (a sidebar, a wizard's "Next" button), use `APageView` and its
`APageSwitch`.

## Splitters

[ASplitter] lets the user redistribute space between views by dragging the border:

```cpp
ASplitter::Horizontal().withItems({ listView, details }).build()
```

Mark one of the items `Expanding`, so there is a view to take the free space.
See the notes example, which has a list of notes and an editor in a splitter.

## Group boxes and dragging

[AGroupBox] draws a titled frame around related controls. [ADragArea] lets the user drag views around inside it:
`ADragArea::convertToDraggable(view, false)` makes a view draggable.
