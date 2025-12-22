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

//
// Created by nelonn on 12/20/25.
//

#include "AWordWrappingEngine.h"

#include <range/v3/all.hpp>
#include <AUI/Util/AFraction.h>

void AWordWrappingEngine::performLayout(const glm::ivec2& offset, const glm::ivec2& size) {
    if (mEntries.empty()) {
        mHeight = 0;
        return;
    }

    struct StandardEntry {
        aui::no_escape<Entry> entry;
        int occupiedHorizontalSpace;
    };

    static constexpr auto UNDEFINED_POSITION_MARKER = std::numeric_limits<int>::min();
    struct FloatingEntry {
        aui::no_escape<Entry> entry;
        int occupiedHorizontalSpace;
        int remainingHeight;
        glm::ivec2 position;

        void setPosition(glm::ivec2 p) {
            position = p;
            if (p.x == UNDEFINED_POSITION_MARKER || p.y == UNDEFINED_POSITION_MARKER) {
                return;
            }
            entry->setPosition(p);
        }
    };



    AVector<AVector<StandardEntry>> inflatedEntriesByRows;
    typename decltype(inflatedEntriesByRows)::iterator currentRow;
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
        const int currentYWithLineHeightApplied = currentY + (mLineHeight - 1.f) / 2.f * currentRowHeight;
        for (AVector<FloatingEntry>* floating : {&leftFloat, &rightFloat}) {
            for (FloatingEntry& i: *floating | ranges::views::reverse) {
                if (i.position.y != UNDEFINED_POSITION_MARKER) {
                    break;
                }
                i.position.y = currentYWithLineHeightApplied;
                i.setPosition(i.position);
            }
        }

        int currentX = 0;
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

                    currentX = offset.x + leftPadding;
                    int index = 0;
                    for (auto& i: *currentRow) {
                        i.entry->setPosition({currentX + (spacing * index).toInt(), currentYWithLineHeightApplied});
                        currentX += i.occupiedHorizontalSpace;
                        ++index;
                    }
                    break;
                }
                // fallthrough
            }
            case ATextAlign::LEFT:
                for (auto& i: leftFloat) {
                    currentX += i.occupiedHorizontalSpace;
                }
                for (auto& i: *currentRow) {
                    i.entry->setPosition({currentX + offset.x, currentYWithLineHeightApplied});
                    currentX += i.occupiedHorizontalSpace;
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

                currentX = leftPadding + (size.x - leftPadding - rightPadding - actualRowWidth) / 2;
                for (auto& i: *currentRow) {
                    i.entry->setPosition({currentX + offset.x, currentYWithLineHeightApplied});
                    currentX += i.occupiedHorizontalSpace;
                }
                break;
            }

            case ATextAlign::RIGHT:
                // calculate actual row width
                int actualRowWidth = 0;
                for (auto& i : *currentRow) actualRowWidth += i.occupiedHorizontalSpace;
                for (auto& i : rightFloat) actualRowWidth += i.occupiedHorizontalSpace;
                currentX = size.x - actualRowWidth;
                for (auto& i : *currentRow) {
                    i.entry->setPosition({currentX + offset.x, currentYWithLineHeightApplied});
                    currentX += i.occupiedHorizontalSpace;
                }
                break;
        }
    };


    beginRow();

    for (auto currentItem = mEntries.begin(); currentItem != mEntries.end(); ++currentItem) {
        auto currentItemSize = (*currentItem)->getSize();
        bool forcesNextLine = (*currentItem)->forcesNextLine();
        bool escapesEdges = (*currentItem)->escapesEdges();

        // check if entry fits into the row
        if (forcesNextLine || (currentRowWidth + currentItemSize.x > size.x && !escapesEdges)) {
            // if current row is empty, we must place this element (unless forcesNextLine)
            if (!currentRow->empty() || forcesNextLine) {
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
            if (mTextAlign == ATextAlign::JUSTIFY && (*currentItem)->escapesEdges()) {
                currentItemSize.x = 0;
            }
            firstItem = false;
        }
        switch ((*currentItem)->getFloat()) {
            case AFloat::LEFT: {
                int position = ranges::accumulate(leftFloat, 0, std::plus<>{}, &FloatingEntry::occupiedHorizontalSpace);
                leftFloat.push_back({*currentItem, currentItemSize.x, currentItemSize.y});
                leftFloat.last().setPosition({position, UNDEFINED_POSITION_MARKER });
                break;
            }

            case AFloat::RIGHT: {
                rightFloat.push_back({*currentItem, currentItemSize.x, currentItemSize.y});
                int position = ranges::accumulate(rightFloat, offset.x + size.x, std::minus<>{}, &FloatingEntry::occupiedHorizontalSpace);
                rightFloat.last().setPosition({position, UNDEFINED_POSITION_MARKER });
                break;
            }

            case AFloat::NONE:
                currentRow->push_back({*currentItem, currentItemSize.x});
                currentRowHeight = glm::max(currentRowHeight, currentItemSize.y);
                break;
        }

        currentRowWidth += currentItemSize.x;
    }
    flushRow(true);

    auto floatingMax = [&] {
        auto r = ranges::views::concat(leftFloat, rightFloat);
        if (r.empty()) {
            return 0;
        }
        return ranges::max(r | ranges::views::transform([](const FloatingEntry& e) { return e.position.y + e.remainingHeight; }));
    }();

    mHeight = std::max(currentY + static_cast<int>(static_cast<float>(currentRowHeight) * mLineHeight), floatingMax) - offset.y;
}
