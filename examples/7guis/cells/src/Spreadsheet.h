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

#pragma once

#include "Cell.h"
#include "AUI/Common/AVector.h"
#include "AUI/Common/AException.h"

class Spreadsheet {
public:
    explicit Spreadsheet(glm::uvec2 size) : mSize(size) {
        mCells.resize(size.x * size.y);
        for (auto& v : mCells) {
            v = std::make_unique<Cell>();
            v->spreadsheet = this;
        }
    }

    Cell& operator[](glm::uvec2 pos) { return *mCells[pos.y * mSize.x + pos.x]; }

    const Cell& operator[](glm::uvec2 pos) const {
        if (glm::any(glm::greaterThanEqual(pos, mSize))) {
            throw AException("OUTOFBOUNDS");
        }
        return *mCells[pos.y * mSize.x + pos.x];
    }

    auto operator[](formula::Range range) const {
        if (range.from.x == Cell::UNDEFINED) {
            range.from.x = 0;
            if (range.to.x != Cell::UNDEFINED) {
                throw AException("BADRANGE");
            }
            range.to.x = mSize.x - 1;
        }
        if (range.from.y == Cell::UNDEFINED) {
            range.from.y = 0;
            if (range.to.y != Cell::UNDEFINED) {
                throw AException("BADRANGE");
            }
            range.to.y = mSize.y - 1;
        }
        if (range.to.x == Cell::UNDEFINED || range.to.y == Cell::UNDEFINED) {
            throw AException("BADRANGE");
        }
        range = { .from = glm::min(range.from, range.to), .to = glm::max(range.from, range.to) };

        struct RangeIterator {
            const Spreadsheet* spreadsheet;
            formula::Range range;
            glm::uvec2 current;

            RangeIterator operator++() {
                current.x += 1;
                if (current.x <= range.to.x) {
                    return *this;
                }
                current.x = range.from.x;
                current.y += 1;
                if (current.y <= range.to.y) {
                    return *this;
                }
                current = { Cell::UNDEFINED, Cell::UNDEFINED };
                return *this;
            }

            const Cell& operator*() const {
                return (*spreadsheet)[current];
            }

            bool operator==(const RangeIterator&) const = default;
            bool operator!=(const RangeIterator&) const = default;
        };
        return aui::range(
            RangeIterator { .spreadsheet = this, .range = range, .current = range.from },
            RangeIterator { .spreadsheet = this, .range = range, .current = { Cell::UNDEFINED, Cell::UNDEFINED } });
    }

    glm::uvec2 size() const { return mSize; }

private:
    glm::uvec2 mSize;
    AVector<_unique<Cell>> mCells;
};
