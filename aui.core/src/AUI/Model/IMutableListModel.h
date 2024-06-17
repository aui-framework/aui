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

#pragma once

#include <AUI/Common/ASignal.h>
#include <AUI/Model/AListModelSelection.h>
#include "AListModelRange.h"

template<typename T>
class IRemovableListModel: public virtual IListModel<T>
{
public:
    virtual ~IRemovableListModel() = default;
    virtual void removeItems(const AListModelRange<T>& items) = 0;
    virtual void removeItems(const AListModelSelection<T>& items) {
        for (const auto& r : items.ranges()) {
            removeItems(r);
        }
    }
    virtual void removeItem(const AListModelIndex& item) = 0;

};


template<typename T>
class IValueMutableListModel: public virtual IListModel<T>
{
public:
    virtual ~IValueMutableListModel() = default;
    virtual void setItem(const AListModelIndex& index, const T& value) = 0;

};
