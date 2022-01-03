#pragma once


#include <AUI/Util/AViewEntry.h>
#include "ALayout.h"

/**
 * Imitates behaviour of word wrapping, but AViews instead words
 */
class API_AUI_VIEWS AWordWrappingLayout: public ALayout {
private:
    AVector<AViewEntry> mViewEntry;

public:
    void onResize(int x, int y, int width, int height) override;

    int getMinimumWidth() override;

    int getMinimumHeight() override;

    void addView(size_t index, const _<AView>& view) override;

    void removeView(size_t index, const _<AView>& view) override;
};


