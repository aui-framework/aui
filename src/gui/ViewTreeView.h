#pragma once

#include <unordered_map>

#include <AUI/Common/AProperty.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AViewContainer.h>

#include "remote/ViewTree.h"

/**
 * @brief View hierarchy of the remote application, as the tree of the in-process devtools (ATreeView logic, without
 * ATreeView).
 * @details
 * The tree is updated with every frame, so the items are kept by ref and reconciled instead of being rebuilt: only the
 * expanded branches are built and refreshed. The window is the root, expanded by default.
 *
 * Hovering an item sets `hovered`, clicking it sets `selected`; both are refs (empty for none).
 */
class ViewTreeView : public AViewContainerBase {
public:
    ViewTreeView();
    ~ViewTreeView() override;

    /**
     * @brief Ref of the view under the pointer; empty if none.
     */
    AProperty<AString> hovered;

    /**
     * @brief Ref of the selected view; empty if none.
     */
    AProperty<AString> selected;

    /**
     * @brief Shows the tree. Null clears it.
     */
    void setTree(_<const aui_remote::ViewTree> tree);

    /**
     * @brief Expands the ancestors of the view, selects it and scrolls to it. Does nothing if the view is not in the
     * tree.
     */
    void select(const AString& ref);

    void render(ARenderContext ctx) override;

private:
    class Item;
    class Row;
    friend class Item;

    _<const aui_remote::ViewTree> mTree;
    _<AScrollArea> mScroll;
    _<AViewContainer> mContent;
    _<Item> mRoot;
    std::unordered_map<std::string, _<Item>> mItems;
    AString mPendingScroll;   // ref of the item to scroll to once it's laid out

    _<Item> itemFor(const AString& ref);
    _<Item> findItem(const AString& ref) const;
    void update(Item& item);
};
