/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

class ViewHierarchyTreeModel : public ITreeModel<AString> {
private:
    _<AViewContainerBase> mRoot;

public:
    ViewHierarchyTreeModel(_<AViewContainerBase> root) : mRoot(std::move(root)) {
        // scan(mRoot); // crashes
    }

    void scan(aui::no_escape<AView> view) {
        auto asContainer = dynamic_cast<AViewContainerBase*>(view.ptr());
        if (!asContainer) {
            return;
        }
        setupConnectionsIfNotPresent(asContainer);

        for (const auto& v : asContainer->getViews()) {
            scan(v);
        }
    }

    void setupConnectionsIfNotPresent(aui::no_escape<AViewContainerBase> container) {
        if (container->childrenChanged.hasOutgoingConnectionsWith(this)) {
            return;
        }

        struct ExtraData {
            AVector<_<AView>> children;
        };

        connect(
            container->childrenChanged, this,
            [this, container = container.ptr(), e = _new<ExtraData>(ExtraData { container->getViews() })]() {
                auto containerIndex = makeIndex(container);

                for (std::size_t i = 0; i < e->children.size(); ++i) {
                    emit dataRemoved(ATreeModelIndex(0, 0, e->children[i]));
                }
                e->children = container->getViews();

                forEachDirectChildOf(containerIndex, [&](const ATreeModelIndex& i) { emit dataInserted(i); });
                scan(container);
            });
    }

    ATreeModelIndexOrRoot makeIndex(aui::no_escape<AView> view) {
        if (view.ptr() == mRoot.get()) {
            return ATreeModelIndex::ROOT;
        }
        std::size_t row = 0;
        auto shared = aui::ptr::shared_from_this(view.ptr());
        if (auto p = view->getParent()) {
            row = p->getViews().indexOf(shared).valueOr(0);
        }
        return ATreeModelIndex(row, 0, shared);
    }

    size_t childrenCount(const ATreeModelIndexOrRoot& vertex) override {
        auto c = vertex == ATreeModelIndex::ROOT ? mRoot : _cast<AViewContainerBase>((*vertex).as<_<AView>>());
        if (c) {
            return c->getViews().size();
        }
        return 0;
    }

    AString itemAt(const ATreeModelIndex& index) override {
        return Devtools::prettyViewName(index.as<_<AView>>().get());
    }

    ATreeModelIndex indexOfChild(size_t row, size_t column, const ATreeModelIndexOrRoot& vertex) override {
        auto c = vertex == ATreeModelIndex::ROOT ? mRoot : _cast<AViewContainerBase>((*vertex).as<_<AView>>());
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

DevtoolsLayoutTab::DevtoolsLayoutTab(ASurface* targetWindow) : mTargetWindow(targetWindow) {
    using namespace declarative;

    setContents(Vertical {
      Horizontal {
        Button { Label { "Force layout update" },  {me::forceLayoutUpdate} },
        SpacerExpanding {},
        Label { "Use CTRL to hit test views" },
      },
      ASplitter::Horizontal()
          .withItems({
            mViewHierarchyTree = _new<ATreeView>() AUI_OVERRIDE_STYLE { MinSize { 300_dp }, Expanding {} },
            Centered { mViewPropertiesView = _new<ViewPropertiesView>(nullptr) },
          })
          .withExpanding().build() AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } });

    auto model = _new<ViewHierarchyTreeModel>(aui::ptr::fake_shared(targetWindow));
    mViewHierarchyTree->setModel(model);
    connect(mViewHierarchyTree->itemSelected, [this](const ATreeModelIndex& index) {
        mViewPropertiesView->setTargetView(index.as<_<AView>>());
    });
    connect(mViewHierarchyTree->itemMouseHover, [this](const ATreeModelIndex& index) {
        if (!AInput::isKeyDown(AInput::LCONTROL)) {
            return;
        }
        mViewPropertiesView->setTargetView(index.as<_<AView>>());
    });
    connect(mouseLeave, [this] {
        AUI_NULLSAFE(mTargetWindow->profiling())->highlightView = _weak<AView>();
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

DevtoolsLayoutTab::~DevtoolsLayoutTab() {
    if (mViewPropertiesView) {
        mViewPropertiesView->setTargetView(nullptr);
    }
    if (mTargetWindow) {
        AUI_NULLSAFE(mTargetWindow->profiling())->highlightView = _weak<AView>();
        mTargetWindow->redraw();
    }
}

void DevtoolsLayoutTab::forceLayoutUpdate() { mTargetWindow->forceUpdateLayoutRecursively(); }
