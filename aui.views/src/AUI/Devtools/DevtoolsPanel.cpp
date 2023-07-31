// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//#ifdef _DEBUG

#include "DevtoolsPanel.h"
#include "AUI/View/ASplitter.h"
#include "Devtools.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/ASS/ASS.h>
#include <AUI/View/AButton.h>


class ViewHierarchyTreeModel: public ITreeModel<AString> {
private:
    _<AView> mRoot;

public:
    ViewHierarchyTreeModel(const _<AView>& root) : mRoot(root) {}

    size_t childrenCount(const ATreeIndex& vertex) override {
        auto c = _cast<AViewContainer>(vertex.as<_<AView>>());
        if (c) {
            return c->getViews().size();
        }
        return 0;
    }

    AString itemAt(const ATreeIndex& index) override {
        return Devtools::prettyViewName(index.as<_<AView>>().get());
    }

    ATreeIndex indexOfChild(size_t row, size_t column, const ATreeIndex& vertex) override {
        auto c = _cast<AViewContainer>(vertex.as<_<AView>>());
        if (!c) {
            throw AException("invalid index");
        }
        return ATreeIndex(row, column, c->getViews().at(row));
    }

    ATreeIndex parent(const ATreeIndex& ofChild) override {
        auto view = ofChild.as<_<AView>>();
        auto parent = view->getParent();
        if (!parent) {
            return {};
        }

        auto parentOfParent = parent->getParent();

        return ATreeIndex{ parentOfParent ? parentOfParent->getViews().indexOf(parent->sharedPtr()) : 0, 0, parent->sharedPtr() };
    }

    ATreeIndex root() override {
        return ATreeIndex(0, 0, mRoot);
    }
};

DevtoolsPanel::DevtoolsPanel(ABaseWindow* targetWindow):
        mTargetWindow(targetWindow) {
    using namespace declarative;

    setContents(Vertical {
            Horizontal {
                    Button { "Force layout update" }.clicked(me::forceLayoutUpdate),
                    SpacerExpanding{},
                    Label { "Use CTRL to hit test views" },
            },
            ASplitter::Horizontal().withItems({
                                                      mViewHierarchyTree = _new<ATreeView>() with_style { ass::MinSize{ 300_dp } },
                                                      mViewPropertiesView = _new<ViewPropertiesView>(nullptr)
                                              }).build() with_style { ass::Expanding{} },
    });
    auto model = _new<ViewHierarchyTreeModel>(aui::ptr::fake(targetWindow));
    mViewHierarchyTree->setModel(model);
    connect(mViewHierarchyTree->itemSelected, [this](const ATreeIndex& index) {
        mViewPropertiesView->setTargetView(index.as<_<AView>>());
    });
    connect(mViewHierarchyTree->itemMouseHover, [this](const ATreeIndex& index) {
        mViewPropertiesView->setTargetView(index.as<_<AView>>());
    });
    connect(mouseLeave, [this] {
        mTargetWindow->setProfiledView(nullptr);
        mTargetWindow->redraw();
    });
    connect(targetWindow->mouseMove, [this, targetWindow, model](glm::ivec2 position) {
        if (!AInput::isKeyDown(AInput::LCONTROL)) {
            return;
        }
        auto mouseOverView = targetWindow->getViewAtRecursive(position);
        if (!mouseOverView) {
            return;
        }

        mViewPropertiesView->setTargetView(mouseOverView);
        auto indexToSelect = model->find([&](const ATreeIndex& index) {
            return index.as<_<AView>>() == mouseOverView;
        });
        if (indexToSelect) {
            mViewHierarchyTree->select(*indexToSelect);
        }
    });
}

void DevtoolsPanel::forceLayoutUpdate() {
    mTargetWindow->updateLayout();
}
