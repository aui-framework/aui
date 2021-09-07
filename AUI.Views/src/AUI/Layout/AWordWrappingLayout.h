#pragma once


#include "ALayout.h"

/**
 * Imitates behaviour of word wrapping, but AViews instead words
 */
class AWordWrappingLayout: public ALayout {
public:
    void onResize(int x, int y, int width, int height) override;

    int getMinimumWidth() override;

    int getMinimumHeight() override;
};


