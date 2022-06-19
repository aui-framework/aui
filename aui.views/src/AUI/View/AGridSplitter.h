#pragma once


#include "AViewContainer.h"
#include "ASplitterHelper.h"
#include <AUI/Util/UIBuildingHelpers.h>

/**
 * @brief A resizable grid layout.
 * @ingroup useful_views
 * @details
 * AGridSplitter represents a grid layout which can be resized by user. Unlike ASplitter, works in both directions.
 * AGridSplitter can be constructed with AGridSplitter::Builder.
 */
class API_AUI_VIEWS AGridSplitter: public AViewContainer {
public:
    class Builder {
        friend class ASplitter;
    private:
        AVector<AVector<_<AView>>> mItems;
        bool mAddSpacers = true;

    public:
        Builder& withItems(const AVector<AVector<_<AView>>>& items) {
            mItems = items;
            return *this;
        }

        Builder& noDefaultSpacers() {
            mAddSpacers = false;
            return *this;
        }

        _<AView> build() {
            auto splitter = aui::ptr::manage(new AGridSplitter);
            if (mAddSpacers) {
                for (auto& row: mItems) {
                    row.push_back(_new<ASpacer>());
                }
                mItems.push_back(
                        AVector<_<AView>>::generate(mItems.first().size(), [](size_t) { return _new<ASpacer>(); }));
            }
            splitter->mItems = std::move(mItems);
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

    AGridSplitter();

public:
    void updateSplitterItems();

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseMove(glm::ivec2 pos) override;

    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    bool consumesClick(const glm::ivec2& pos) override;
};


