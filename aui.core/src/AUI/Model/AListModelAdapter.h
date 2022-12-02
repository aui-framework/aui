// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <AUI/Model/IListModel.h>
#include <AUI/Model/IMutableListModel.h>

template<typename ItemTo, typename ItemFrom, typename Adapter>
class AListModelAdapter: public IRemovableListModel<ItemTo>, public AObject {
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
        AObject::connect(other->dataChanged, this, [&](const AModelRange<ItemFrom>& r){
            emit this->dataChanged({r.getBegin(), r.getEnd(), this});
        });
        AObject::connect(other->dataInserted, this, [&](const AModelRange<ItemFrom>& r){
            emit this->dataInserted({r.getBegin(), r.getEnd(), this});
        });
        AObject::connect(other->dataRemoved, this, [&](const AModelRange<ItemFrom>& r){
            emit this->dataRemoved({r.getBegin(), r.getEnd(), this});
        });
    }

    ~AListModelAdapter() override = default;

    size_t listSize() override {
        return mOther->listSize();
    }

    ItemTo listItemAt(const AModelIndex& index) override {
        return mAdapter(mOther->listItemAt(index));
    }


    void removeItems(const AModelRange<ItemTo>& items) override {
        AUI_NULLSAFE(mOtherMutable)->removeItems({items.begin().getIndex(), items.end().getIndex(), mOther.get()});
    }

    void removeItems(const AModelSelection<ItemTo>& items) override {
        AUI_NULLSAFE(mOtherMutable)->removeItems({items.getIndices(), mOther.get()});
    }

    void removeItem(const AModelIndex& item) override {
        AUI_NULLSAFE(mOtherMutable)->removeItem(item);
    }

};

namespace AModels {
    template <typename ItemTo, typename Container, typename Adapter>
    auto adapt(const _<Container>& other, Adapter&& adapter) -> _<AListModelAdapter<ItemTo, typename Container::value_type, Adapter>> {
        return aui::ptr::manage(new AListModelAdapter<ItemTo, typename Container::value_type, Adapter>(other, std::forward<Adapter>(adapter)));
    }
}
