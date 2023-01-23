// AUI Framework - Declarative UI toolkit for modern C++17
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

    size_t childrenCount(const ATreeIndex& parent) override {
        auto c = dynamic_cast<AViewContainer*>(parent.getUserData<_<AView>>()->get());
        if (c) {
            return c->getViews().size();
        }
        return 0;
    }

    AString itemAt(const ATreeIndex& index) override {
        return Devtools::prettyViewName(index.getUserData<_<AView>>()->get());
    }

    ATreeIndex indexOfChild(size_t row, size_t column, const ATreeIndex& parent) override {
        auto ptr = const_cast<_<AView>*>(&dynamic_cast<AViewContainer*>(parent.getUserData<_<AView>>()->get())->getViews()[row]);
        return ATreeIndex(ptr);
    }

protected:
    void* rootUserData() override {
        return &mRoot;
    }
};

DevtoolsPanel::DevtoolsPanel(ABaseWindow* targetWindow):
        mTargetWindow(targetWindow) {
    using namespace declarative;

    setContents(Vertical {
            Horizontal {
                    Button { "Force layout update" }.clicked(me::forceLayoutUpdate)
            },
            ASplitter::Horizontal().withItems({
                                                      mViewHierarchyTree = _new<ATreeView>() with_style { ass::MinSize{ 300_dp } },
                                                      mViewPropertiesView = _new<ViewPropertiesView>(nullptr)
                                              }).build() with_style { ass::Expanding{} },
    });
    mViewHierarchyTree->setModel(_new<ViewHierarchyTreeModel>(aui::ptr::fake(targetWindow)));
    connect(mViewHierarchyTree->itemMouseClicked, [this](const ATreeIndex& index) {
        mViewPropertiesView->setTargetView(*index.getUserData<_<AView>>());
    });
    connect(mViewHierarchyTree->itemMouseHover, [this](const ATreeIndex& index) {
        mTargetWindow->setProfiledView(*index.getUserData<_<AView>>());
        mTargetWindow->redraw();
    });
    connect(mouseLeave, [this] {
        mTargetWindow->setProfiledView(nullptr);
        mTargetWindow->redraw();
    });
}

void DevtoolsPanel::forceLayoutUpdate() {
    mTargetWindow->updateLayout();
}
