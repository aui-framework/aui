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

#include <AUI/Model/IListModel.h>
#include <AUI/Model/IMutableListModel.h>

template<typename ItemTo, typename ItemFrom, typename Adapter>
class AListModelAdapter: public IRemovableListModel<ItemTo> {
private:
    _<IListModel<ItemFrom>> mOther;
    IRemovableListModel<ItemFrom>* mOtherMutable;
    Adapter mAdapter;

public:
    using value_type = ItemTo;

    explicit AListModelAdapter(const _<IListModel<ItemFrom>>& other, Adapter&& adapter) :
            mOther(other),
            mAdapter(std::forward<Adapter>(adapter)) {
        mOtherMutable = dynamic_cast<IRemovableListModel<ItemFrom>*>(mOther.get());
        AObject::connect(other->dataChanged, this, [&](const AListModelRange<ItemFrom>& r){
            emit this->dataChanged(this->range(r.getBegin(), r.getEnd()));
        });
        AObject::connect(other->dataInserted, this, [&](const AListModelRange<ItemFrom>& r){
            emit this->dataInserted(this->range(r.getBegin(), r.getEnd()));
        });
        AObject::connect(other->dataRemoved, this, [&](const AListModelRange<ItemFrom>& r){
            emit this->dataRemoved(this->range(r.getBegin(), r.getEnd()));
        });
    }

    ~AListModelAdapter() override = default;

    size_t listSize() override {
        return mOther->listSize();
    }

    ItemTo listItemAt(const AListModelIndex& index) override {
        return mAdapter(mOther->listItemAt(index));
    }


    void removeItems(const AListModelRange<ItemTo>& items) override {
        AUI_NULLSAFE(mOtherMutable)->removeItems({items.begin().getIndex(), items.end().getIndex(), mOther});
    }

    void removeItems(const AListModelSelection<ItemTo>& items) override {
        AUI_NULLSAFE(mOtherMutable)->removeItems({items.getIndices(), mOther});
    }

    void removeItem(const AListModelIndex& item) override {
        AUI_NULLSAFE(mOtherMutable)->removeItem(item);
    }

};

namespace AModels {
    template <typename ItemTo, typename Container, typename Adapter>
    auto adapt(const _<Container>& other, Adapter&& adapter) -> _<AListModelAdapter<ItemTo, typename Container::value_type, Adapter>> {
        return aui::ptr::manage_shared(new AListModelAdapter<ItemTo, typename Container::value_type, Adapter>(other, std::forward<Adapter>(adapter)));
    }
}
