#pragma once


#include "AViewContainer.h"
#include "ASplitterHelper.h"
#include <AUI/Util/UIBuildingHelpers.h>

class API_AUI_VIEWS AGridSplitter: public AViewContainer {
public:
    class Builder {
        friend class ASplitter;
    private:
        AVector<AVector<_<AView>>> mItems;
        bool mCrosses = false;

    public:
        Builder& withItems(const AVector<AVector<_<AView>>>& items) {
            mItems = items;
            return *this;
        }

        Builder& withCrosses() {
            mCrosses = true;
            return *this;
        }

        _<AView> build() {
            _<AGridSplitter> splitter = new AGridSplitter;
            for (auto& row : mItems) {
                row.push_back(_new<ASpacer>());
            }
            mItems.push_back(AVector<_<AView>>::generate(mItems.first().size(), [](size_t){ return _new<ASpacer>(); }));
            splitter->mItems = std::move(mItems);
            splitter->mCrosses = mCrosses;
            splitter->updateSplitterItems();
            return splitter;
        }

        operator _<AView>() {
            return build();
        }
    };

private:
    friend class Builder;
    AVector<AVector<_<AView>>> mItems;
    ASplitterHelper mHorizontalHelper;
    ASplitterHelper mVerticalHelper;
    bool mCrosses;

    AGridSplitter();

public:
    void updateSplitterItems();

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseMove(glm::ivec2 pos) override;

    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;
};


