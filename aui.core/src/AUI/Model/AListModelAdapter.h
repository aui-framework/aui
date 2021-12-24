/**
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

template<typename T, typename Adapter> class AListModelAdapter;

struct AAdapter {
    template<typename T, typename Adapter>
    static _<AListModelAdapter<T, Adapter>> make(const _<IListModel<T>>& other, const Adapter& adapter);
};

template<typename T, typename Adapter>
class AListModelAdapter: public IMutableListModel<AString>, public AObject {
friend struct AAdapter;
private:
    _<IListModel<T>> mOther;
    IMutableListModel<T>* mOtherMutable;
    Adapter mAdapter;

    explicit AListModelAdapter(const _<IListModel<T>>& other, const Adapter& adapter) :
            mOther(other),
            mAdapter(adapter) {
        mOtherMutable = dynamic_cast<IMutableListModel<T>*>(mOther.get());
        AObject::connect(other->dataChanged, this, [&](const AModelRange<T>& r){
            emit dataChanged({r.getBegin(), r.getEnd(), this});
        });
        AObject::connect(other->dataInserted, this, [&](const AModelRange<T>& r){
            emit dataInserted({r.getBegin(), r.getEnd(), this});
        });
        AObject::connect(other->dataRemoved, this, [&](const AModelRange<T>& r){
            emit dataRemoved({r.getBegin(), r.getEnd(), this});
        });
    }

public:
    ~AListModelAdapter() override = default;

    size_t listSize() override {
        return mOther->listSize();
    }

    AString listItemAt(const AModelIndex& index) override {
        return mAdapter(mOther->listItemAt(index));
    }

    void removeItems(const AModelRange<AString>& items) override {
        nullsafe(mOtherMutable)->removeItems({items.begin().getIndex(), items.end().getIndex(), mOther.get()});
    }

    void removeItems(const AModelSelection<AString>& items) override {
        nullsafe(mOtherMutable)->removeItems({items.getIndices(), mOther.get()});
    }

    void removeItem(const AModelIndex& item) override {
        nullsafe(mOtherMutable)->removeItem(item);
    }

};

template <typename T, typename Adapter>
_<AListModelAdapter<T, Adapter>> AAdapter::make(const _<IListModel<T>>& other, const Adapter& adapter) {
    return aui::ptr::manage(new AListModelAdapter<T, Adapter>(other, adapter));
}