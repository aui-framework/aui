#pragma once


#include <AUI/Util/AViewEntry.h>
#include "ALayout.h"

/**
 * @brief Imitates behaviour of word wrapping, but uses @ref AView "views" instead words
 * @ingroup layout_managers
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


