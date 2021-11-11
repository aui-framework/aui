//#ifdef _DEBUG

#include "DevtoolsWindow.h"
#include "AUI/View/ASplitter.h"
#include "Devtools.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/ASS/ASS.h>


class ViewHierarchyTreeModel: public ITreeModel<AString> {
private:
    AView* mRoot;

public:
    ViewHierarchyTreeModel(AView* root) : mRoot(root) {}

    size_t childrenCount(const ATreeIndex& parent) override {
        auto c = dynamic_cast<AViewContainer*>(parent.getUserData<AView>());
        if (c) {
            return c->getViews().size();
        }
        return 0;
    }

    AString itemAt(const ATreeIndex& index) override {
        return Devtools::prettyViewName(index.getUserData<AView>());
    }

    ATreeIndex indexOfChild(size_t row, size_t column, const ATreeIndex& parent) override {
        return ATreeIndex(dynamic_cast<AViewContainer*>(parent.getUserData<AView>())->getViews()[row].get());
    }

protected:
    void* rootUserData() override {
        return mRoot;
    }
};

DevtoolsWindow::DevtoolsWindow(ABaseWindow* targetWindow):
        AWindow("Devtools", 600_dp, 400_dp),
        mTargetWindow(targetWindow) {

    setContents(Stacked {
        ASplitter::Horizontal().withItems({
            mViewHierarchyTree = _new<ATreeView>() with_style { ass::MinSize{ 300_dp } },
            mViewPropertiesView = _new<ViewPropertiesView>(targetWindow)
        }).build() with_style { ass::Expanding{} },
    });
    mViewHierarchyTree->setModel(_new<ViewHierarchyTreeModel>(targetWindow));
    connect(mViewHierarchyTree->itemMouseClicked, [this](const ATreeIndex& index) {
        mViewPropertiesView->setTargetView(index.getUserData<AView>());
    });
}



//#endif
