/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by Alex2772 on 11/16/2021.
//

#include <range/v3/numeric/accumulate.hpp>
#include <AUI/Traits/iterators.h>
#include "AWordWrappingEngine.h"
#include <algorithm>
#include <numeric>
#include <AUI/Util/AFraction.h>

void AWordWrappingEngine::performLayout(const glm::ivec2& offset, const glm::ivec2& size) {
    if (mEntries.empty()) return;


    struct StandardEntry {
        _<Entry> entry;
        int occupiedHorizontalSpace;
    };

    struct FloatingEntry {
        _<Entry> entry;
        int occupiedHorizontalSpace;
        int remainingHeight;
    };


    AVector<AVector<StandardEntry>> inflatedEntriesByRows;
    decltype(inflatedEntriesByRows)::iterator currentRow;
    size_t currentRowWidth = 0;
    int currentRowHeight = 0;
    int currentY = offset.y;

    AVector<FloatingEntry> leftFloat;
    AVector<FloatingEntry> rightFloat;

    bool firstItem;

    auto beginRow = [&] {
        firstItem = true;
        currentRowWidth = 0;
        for (auto& i : leftFloat) {
            currentRowWidth += i.occupiedHorizontalSpace;
        }
        for (auto& i : rightFloat) {
            currentRowWidth += i.occupiedHorizontalSpace;
        }
        inflatedEntriesByRows.push_back({});
        currentRow = inflatedEntriesByRows.end() - 1;
    };

    auto flushRow = [&](bool last) {
        int currentPos = 0;
        switch (mTextAlign) {
            case ATextAlign::JUSTIFY: {
                if (!last) {
                    int actualRowWidth = 0;
                    int leftPadding = 0;
                    int rightPadding = 0;

                    for (auto& i: leftFloat) leftPadding += i.occupiedHorizontalSpace;
                    for (auto& i: rightFloat) rightPadding += i.occupiedHorizontalSpace;

                    if (!currentRow->empty()) {
                        if (currentRow->last().entry->escapesEdges()) {
                            for (auto it = currentRow->begin(); it != currentRow->end() - 1; ++it) {
                                actualRowWidth += it->occupiedHorizontalSpace;
                            }
                        } else {
                            for (auto& i: *currentRow) actualRowWidth += i.occupiedHorizontalSpace;
                        }
                    }

                    int freeSpace = size.x - leftPadding - rightPadding;

                    AFraction spacing(freeSpace - actualRowWidth, (glm::max)(int(currentRow->size()) - 1, 1));

                    currentPos = offset.x + leftPadding;
                    int index = 0;
                    for (auto& i: *currentRow) {
                        i.entry->setPosition({currentPos + (spacing * index).toInt(), currentY});
                        currentPos += i.occupiedHorizontalSpace;
                        ++index;
                    }
                    break;
                }
                // fallthrough
            }
            case ATextAlign::LEFT:
                for (auto& i: leftFloat) {
                    currentPos += i.occupiedHorizontalSpace;
                }
                for (auto& i: *currentRow) {
                    i.entry->setPosition({currentPos + offset.x, currentY});
                    currentPos += i.occupiedHorizontalSpace;
                }
                break;

            case ATextAlign::CENTER: {
                int actualRowWidth = 0;
                int leftPadding = 0;
                int rightPadding = 0;

                for (auto& i: leftFloat) leftPadding += i.occupiedHorizontalSpace;
                for (auto& i: rightFloat) rightPadding += i.occupiedHorizontalSpace;
                if (!currentRow->empty()) {
                    if (currentRow->last().entry->escapesEdges()) {
                        for (auto it = currentRow->begin(); it != currentRow->end() - 1; ++it) {
                            actualRowWidth += it->occupiedHorizontalSpace;
                        }
                    } else {
                        for (auto& i: *currentRow) actualRowWidth += i.occupiedHorizontalSpace;
                    }
                }

                currentPos = leftPadding + (size.x - leftPadding - rightPadding - actualRowWidth) / 2;
                for (auto& i: *currentRow) {
                    i.entry->setPosition({currentPos + offset.x, currentY});
                    currentPos += i.occupiedHorizontalSpace;
                }
                break;
            }

            case ATextAlign::RIGHT:
                // calculate actual row width
                int actualRowWidth = 0;
                for (auto& i : *currentRow) actualRowWidth += i.occupiedHorizontalSpace;
                for (auto& i : rightFloat) actualRowWidth += i.occupiedHorizontalSpace;
                currentPos = size.x - actualRowWidth;
                for (auto& i : *currentRow) {
                    i.entry->setPosition({currentPos + offset.x, currentY});
                    currentPos += i.occupiedHorizontalSpace;
                }
                break;
        }
    };


    beginRow();

    for (auto currentItem = mEntries.begin(); currentItem != mEntries.end(); ++currentItem) {
        auto currentItemSize = (*currentItem)->getSize();

        // check if entry fits into the row
        if (currentRowWidth + currentItemSize.x  > size.x) {
            // if current row is empty, we must place this element in any case
            if (!currentRow->empty()) {
                // jump to the next row

                auto removeRedundantItems = [&currentRowHeight](AVector<FloatingEntry>& fl) {
                    for (auto it = fl.begin(); it != fl.end();) {
                        it->remainingHeight -= currentRowHeight;
                        if (it->remainingHeight <= 0) {
                            it = fl.erase(it);
                        } else {
                            ++it;
                        }
                    }
                };

                flushRow(false);

                removeRedundantItems(leftFloat);
                removeRedundantItems(rightFloat);

                currentY += int(float(currentRowHeight) * mLineHeight);
                currentRowHeight = 0;
                beginRow();
            }
        }
        if (firstItem) {
            if ((*currentItem)->escapesEdges()) {
                currentItemSize.x = 0;
            }
            firstItem = false;
        }
        switch ((*currentItem)->getFloat()) {
            case AFloat::LEFT: {
                leftFloat.push_back({*currentItem, currentItemSize.x, currentItemSize.y});
                int position = ranges::accumulate(leftFloat, 0, std::plus<>{}, &FloatingEntry::occupiedHorizontalSpace);
                (*currentItem)->setPosition({position, currentY});
                currentRowHeight = glm::max(currentRowHeight, currentItemSize.y);
                break;
            }

            case AFloat::RIGHT: {
                rightFloat.push_back({*currentItem, currentItemSize.x, currentItemSize.y});
                int position = ranges::accumulate(rightFloat, offset.x + size.x, std::minus<>{}, &FloatingEntry::occupiedHorizontalSpace);
                (*currentItem)->setPosition({position, currentY});
                currentRowHeight = glm::max(currentRowHeight, currentItemSize.y);
                break;
            }

            case AFloat::NONE:
                currentRow->push_back({*currentItem, currentItemSize.x});
                currentRowHeight = glm::max(currentRowHeight, currentItemSize.y);
                break;
        }

        currentRowWidth += currentItemSize.x;
    }
    if (!currentRow->empty()) {
        flushRow(true);
    }

    mHeight = currentY + int(float(currentRowHeight) * mLineHeight) - offset.y;
}
