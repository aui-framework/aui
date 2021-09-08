//
// Created by Alex2772 on 9/7/2021.
//

#include "AWordWrappingLayout.h"

int AWordWrappingLayout::getMinimumWidth() {
    int m = 0;
    for (auto& v : mViews) {
        m = glm::max(m, int(v->getTotalOccupiedWidth()));
    }
    return m;
}

int AWordWrappingLayout::getMinimumHeight() {
    if (mViews.empty()) {
        return 0;
    }
    auto& last = mViews.last();
    return last->getPosition().y + last->getTotalOccupiedHeight();
}

void AWordWrappingLayout::onResize(int x, int y, int width, int height) {
    int currentRowWidth = 0;
    int currentRowY = y;
    auto currentRowFirstView = mViews.begin();
    for (auto it = mViews.begin(); ; ++it) {
        int currentViewOccupiedWidth = it == mViews.end() ? 0 : (*it)->getTotalOccupiedWidth();
        if (currentRowWidth + currentViewOccupiedWidth >= width || (it == mViews.end() && currentRowFirstView != it)) {
            // flush the row
            int rowHeight = 0;

            // calculate rowHeight
            for (auto j = currentRowFirstView; j != it; ++j) {
                rowHeight = glm::max(rowHeight, int((*j)->getMinimumHeight() + (*j)->getTotalFieldVertical()));
            }

            // apply layout
            int currentX = x;
            for (auto j = currentRowFirstView; j != it; ++j) {
                auto margins = (*j)->getMargin();
                (*j)->setGeometry(currentX + margins.left,
                                  currentRowY + margins.top,
                                  (*j)->getMinimumWidth() + (*j)->getPadding().horizontal(),
                                  rowHeight - margins.vertical());
                currentX += (*j)->getTotalOccupiedWidth();
            }
            currentRowFirstView = it;
            currentRowY += rowHeight;
            currentRowWidth = 0;
        }
        if (it == mViews.end()) {
            break;
        }
        currentRowWidth += currentViewOccupiedWidth;
    }
}
