/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
