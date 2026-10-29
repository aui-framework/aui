#include "ViewTreeView.h"

#include <AUI/ASS/ASS.h>
#include <AUI/ASS/Selector/Selected.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/ALabel.h>

using namespace declarative;
using namespace aui_remote;

namespace {

constexpr std::size_t MAX_TEXT = 40;

AString title(const AJson& fields) {
    auto name = ViewTree::prettyName(fields);
    if (auto text = fields.containsOpt("text"); text && text->isString() && !text->asString().empty()) {
        auto s = text->asString().replacedAll("\n", " ");
        if (s.length() > MAX_TEXT) {
            s = s.substr(0, MAX_TEXT) + "...";
        }
        name += " \"" + s + "\"";
    }
    return name;
}

}   // namespace

/// The line of an item: the expand arrow and the name. Selectable, so the `.list-item` styles of ATreeView apply.
class ViewTreeView::Row : public AViewContainerBase, public ass::ISelectable {
public:
    Row(ViewTreeView& tree, AString ref, _<AViewContainer> contents) : mTree(tree), mRef(std::move(ref)) {
        addAssName(".list-item");
        setContents(std::move(contents));
    }

    void selectionChanged() { emit customCssPropertyChanged; }

protected:
    bool selectableIsSelectedImpl() override { return *mTree.selected == mRef; }

private:
    ViewTreeView& mTree;
    AString mRef;
};

/// A view of the tree with the container of its children, which is filled once the item is expanded.
class ViewTreeView::Item : public AViewContainerBase {
public:
    Item(ViewTreeView& tree, AString ref) : mTree(tree), ref(std::move(ref)) {
        row = _new<Row>(
            tree, this->ref,
            Horizontal {
              Icon { AUI_REACT(*expanded ? ":uni/svg/tree-expanded.svg" : ":uni/svg/tree-collapsed.svg") } AUI_LET {
                  it << ".list-item-icon";
                  connect(AUI_REACT(*hasChildren ? Visibility::VISIBLE : Visibility::INVISIBLE), it->visibility());
                  connect(it->clicked, me::toggle);
              },
              Label { AUI_REACT(*title) },
            });
        group = Vertical {} << ".list-item-group";
        connect(AUI_REACT(*expanded ? Visibility::VISIBLE : Visibility::GONE), group->visibility());

        connect(row->clicked, [this] { mTree.selected = this->ref; });
        connect(row->doubleClicked, me::toggle);
        connect(row->mouseEnter, [this] { mTree.hovered = this->ref; });
        connect(row->mouseLeave, [this] {
            if (*mTree.hovered == this->ref) {
                mTree.hovered = "";
            }
        });
        connect(expanded.changed, [this](bool expanded) {
            if (expanded) {
                mTree.update(*this);
            }
        });

        setContents(Vertical { row, group });
    }

    void toggle() { expanded = !*expanded; }

    ViewTreeView& mTree;
    const AString ref;
    AProperty<bool> expanded = false;
    AProperty<bool> hasChildren = false;
    AProperty<AString> title;
    AVector<AString> shownChildren;   // refs of the items in `group`
    _<Row> row;
    _<AViewContainer> group;
};

ViewTreeView::ViewTreeView() {
    mContent = Vertical {};
    mScroll = AScrollArea::Builder().withContents(mContent).build();
    mScroll->setExpanding();
    setContents(Stacked { mScroll });

    connect(selected.changed, [this, shown = AString {}](const AString& ref) mutable {
        if (auto item = findItem(shown)) {
            item->row->selectionChanged();
        }
        shown = ref;
        if (auto item = findItem(ref)) {
            item->row->selectionChanged();
        }
    });
}

ViewTreeView::~ViewTreeView() = default;

_<ViewTreeView::Item> ViewTreeView::findItem(const AString& ref) const {
    auto it = mItems.find(ref.toStdString());
    return it == mItems.end() ? nullptr : it->second;
}

_<ViewTreeView::Item> ViewTreeView::itemFor(const AString& ref) {
    auto& item = mItems[ref.toStdString()];
    if (!item) {
        item = _new<Item>(*this, ref);
    }
    return item;
}

void ViewTreeView::setTree(_<const ViewTree> tree) {
    mTree = std::move(tree);
    if (!mTree || mTree->empty()) {
        mContent->setViews({});
        mRoot = nullptr;
        mItems.clear();
        hovered = "";
        selected = "";
        return;
    }

    // forget the views that are gone; their items may still be shown until their parents are updated below
    std::erase_if(mItems, [&](const auto& entry) { return !mTree->fields(AString::fromUtf8(entry.first)); });

    if (auto root = itemFor(mTree->rootRef()); root != mRoot) {
        mRoot = root;
        mContent->setViews({ root });
        root->expanded = true;   // updates it
    }
    update(*mRoot);

    if (!mTree->fields(*hovered)) {
        hovered = "";
    }
    if (!mTree->fields(*selected)) {
        selected = "";
    }
}

void ViewTreeView::update(Item& item) {
    if (!mTree) {
        return;
    }
    const auto* fields = mTree->fields(item.ref);
    if (!fields) {
        return;
    }
    if (auto t = title(*fields); *item.title != t) {
        item.title = std::move(t);
    }
    const auto& children = mTree->children(item.ref);
    if (*item.hasChildren != !children.empty()) {
        item.hasChildren = !children.empty();
    }
    if (!*item.expanded) {
        return;   // built when expanded
    }
    if (item.shownChildren != children) {
        AVector<_<AView>> views;
        views.reserve(children.size());
        for (const auto& child : children) {
            auto childItem = itemFor(child);
            if (auto parent = dynamic_cast<AViewContainer*>(childItem->getParent()); parent && parent != item.group.get()) {
                parent->removeView(childItem);   // the view has moved
            }
            views << childItem;
        }
        item.group->setViews(std::move(views));
        item.shownChildren = children;
    }
    for (const auto& child : children) {
        update(*itemFor(child));
    }
}

void ViewTreeView::select(const AString& ref) {
    if (!mTree || *selected == ref) {
        return;
    }
    auto path = mTree->pathTo(ref);
    if (path.empty()) {
        return;
    }
    path.pop_back();
    for (const auto& ancestor : path) {
        itemFor(ancestor)->expanded = true;
    }
    selected = ref;
    mPendingScroll = ref;
    redraw();
}

void ViewTreeView::render(ARenderContext ctx) {
    AViewContainerBase::render(ctx);
    // the items of a newly expanded branch get their positions with the layout, which is done by now
    if (auto item = findItem(std::exchange(mPendingScroll, {}))) {
        // vertically only: rows are wider than the panel, scrolling to their right end hides the names
        mScroll->scrollTo(ARect<int>::fromTopLeftPositionAndSize(
            { mScroll->getPositionInWindow().x, item->row->getPositionInWindow().y }, { 1, item->row->getHeight() }));
    }
}
