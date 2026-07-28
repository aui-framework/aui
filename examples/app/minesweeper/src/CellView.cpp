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

#include "CellView.h"

#include "AUI/Render/IRenderer.h"

CellView::CellView(FieldCell& cell) : mCell(cell), mCellValueCopy(cell) {
    connect(clickedButton, this, [&]() {
        emit customCssPropertyChanged();
    });
}

void CellView::render(ARenderContext context) {
    if (mCell != mCellValueCopy) {
        mCellValueCopy = mCell;
    }
    AView::render(context);

    if (bool(mCell & FieldCell::OPEN)) {
        int count = field_cell::getBombCountAround(mCell);
        if (count) {
            AFontStyle fs;
            fs.size = getHeight() * 6 / 7;
            fs.align = ATextAlign::CENTER;
            auto color = AColor::BLACK;

            switch (count) {
                case 1:
                    color = 0x0000ffffu;
                    break;
                case 2:
                    color = 0x008000ffu;
                    break;
                case 3:
                    color = 0xff0000ffu;
                    break;
                case 4:
                    color = 0x000080ffu;
                    break;
                case 5:
                    color = 0x800000ffu;
                    break;
                case 6:
                    color = 0x008080ffu;
                    break;
                case 7:
                    color = 0x000000ffu;
                    break;
                case 8:
                    color = 0x808080ffu;
                    break;
            }

            context.render.setColor(color);
            context.render.string({getWidth() / 3, (getHeight() - fs.size) / 2 + fs.getAscenderHeight()}, AString::number(count), fs);
        }
    }
}
