// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "DevtoolsLayoutTab.h"
#include "AUI/Common/AObject.h"
#include "AUI/Model/ATreeModelIndex.h"
#include "AUI/Model/ITreeModel.h"
#include "AUI/Traits/values.h"
#include "AUI/View/ASplitter.h"
#include "AUI/View/ATabView.h"
#include "AUI/View/AViewContainer.h"
#include "Devtools.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/ASS/ASS.h>
#include <AUI/View/AButton.h>

class ViewHierarchyTreeModel: public ITreeModel<AString> {
private:
    _<AViewContainer> mRoot;

public:
    ViewHierarchyTreeModel(_<AViewContainer> root) : mRoot(std::move(root)) {
        // scan(mRoot); // crashes
    }

    void scan(aui::no_escape<AView> view) {
        auto asContainer = dynamic_cast<AViewContainer*>(view.ptr());
        if (!asContainer) {
            return;
        }
        setupConnectionsIfNotPresent(asContainer);

        for (const auto& v : asContainer->getViews()) {
            scan(v);
        }
    }

    void setupConnectionsIfNotPresent(aui::no_escape<AViewContainer> container) {
        if (container->childrenChanged.hasConnectionsWith(this)) {
            return;
        }

        struct ExtraData {
            AVector<_<AView>> children;
        };
        
        connect(container->childrenChanged, this, [this, container = container.ptr(), e = _new<ExtraData>(ExtraData{container->getViews()})]() {
            auto containerIndex = makeIndex(container);

            for (std::size_t i = 0; i < e->children.size(); ++i) {
                emit dataRemoved(ATreeModelIndex(0, 0, e->children[i]));
            }
            e->children = container->getViews();

            forEachDirectChildOf(containerIndex, [&](const ATreeModelIndex& i) {
                emit dataInserted(i);
            });
            scan(container);
        });
    }

    ATreeModelIndexOrRoot makeIndex(aui::no_escape<AView> view) {
        if (view.ptr() == mRoot.get()) {
            return ATreeModelIndex::ROOT;
        }
        std::size_t row = 0;
        if (auto p = view->getParent()) {
            row = p->getViews().indexOf(view->sharedPtr());
        }
        return ATreeModelIndex(row, 0, view->sharedPtr());
    }

    size_t childrenCount(const ATreeModelIndexOrRoot& vertex) override {
        auto c = vertex == ATreeModelIndex::ROOT ? mRoot : _cast<AViewContainer>((*vertex).as<_<AView>>());
        if (c) {
            return c->getViews().size();
        }
        return 0;
    }

    AString itemAt(const ATreeModelIndex& index) override {
        return Devtools::prettyViewName(index.as<_<AView>>().get());
    }

    ATreeModelIndex indexOfChild(size_t row, size_t column, const ATreeModelIndexOrRoot& vertex) override {
        auto c = vertex == ATreeModelIndex::ROOT ? mRoot : _cast<AViewContainer>((*vertex).as<_<AView>>());
        if (!c) {
            throw AException("invalid index");
        }
        return ATreeModelIndex(row, column, c->getViews().at(row));
    }

    ATreeModelIndexOrRoot parent(const ATreeModelIndex& ofChild) override {
        auto view = ofChild.as<_<AView>>();
        auto parent = view->getParent();

        if (!parent) {
            // window
            return ATreeModelIndex::ROOT;
        }

        return makeIndex(parent);
    }
};

DevtoolsLayoutTab::DevtoolsLayoutTab(ABaseWindow* targetWindow):
        mTargetWindow(targetWindow) {
    using namespace declarative;

    setContents(
        Vertical {
            Horizontal {
                Button { "Force layout update" }.clicked(me::forceLayoutUpdate),
                SpacerExpanding{},
                Label { "Use CTRL to hit test views" },
            },
            Horizontal::Expanding{
                mViewHierarchyTree = _new<ATreeView>() with_style{ass::MinSize{300_dp}, ass::Expanding{}},
                mViewPropertiesView = _new<ViewPropertiesView>(nullptr),
            }, 
        }
    );


    auto model = _new<ViewHierarchyTreeModel>(aui::ptr::fake(targetWindow));
    mViewHierarchyTree->setModel(model);
    connect(mViewHierarchyTree->itemSelected, [this](const ATreeModelIndex& index) {
        mViewPropertiesView->setTargetView(index.as<_<AView>>());
    });
    connect(mViewHierarchyTree->itemMouseHover, [this](const ATreeModelIndex& index) {
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
        auto indexToSelect = model->find([&](const ATreeModelIndex& index) {
            return index.as<_<AView>>() == mouseOverView;
        });
        if (indexToSelect) {
            mViewHierarchyTree->select(*indexToSelect);
        }
    });
}

void DevtoolsLayoutTab::forceLayoutUpdate() {
    mTargetWindow->updateLayout();
}