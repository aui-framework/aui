#pragma once

#include <AUI/Model/IListModel.h>

template<typename T, typename Adapter>
class AListModelAdapter: public IListModel<AString>, public AObject {
private:
    _<IListModel<T>> mOther;
    Adapter mAdapter;

public:
    explicit AListModelAdapter(const _<IListModel<T>>& other, const Adapter& adapter) :
        mOther(other),
        mAdapter(adapter) {

        AObject::connect(other->dataChanged, this, [&](const AModelRange<T>& r){
           emit dataChanged({r.getBegin(), r.getEnd(), this});
        });
        AObject::connect(other->dataInserted, this, [&](const AModelRange<T>& r){
            emit dataInserted({r.getBegin(), r.getEnd(), this});
        });
    }

    ~AListModelAdapter() override = default;

    size_t listSize() override {
        return mOther->listSize();
    }

    AString listItemAt(const AModelIndex& index) override {
        return mAdapter(mOther->listItemAt(index));
    }
};

namespace AAdapter {
    template <typename T, typename Adapter>
    static _<AListModelAdapter<T, Adapter>> make(const _<IListModel<T>>& other, const Adapter& adapter) {
        return _new<AListModelAdapter<T, Adapter>>(other, adapter);
    }
}

