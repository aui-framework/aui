#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/Common/ASignal.h>
#include "AModelIndex.h"
#include "IListModel.h"

template <typename StoredType>
class AListModel: public AVector<StoredType>, public AObject, public IListModel<StoredType> {
    using p = AVector<StoredType>;
public:

    inline AListModel() = default;


    inline AListModel(typename p::size_type _Count)
            : p(_Count)
    {
    }

    inline AListModel(typename p::size_type _Count, const StoredType& _Val)
            : p(_Count, _Val)
    {
    }

    inline AListModel(std::initializer_list<StoredType> _Ilist)
    : p(_Ilist)
    {
    }

    inline AListModel(const p& _Right)
            : p(_Right)
    {
    }
    inline AListModel(const AListModel<StoredType>& _Right)
            : p(_Right.begin(), _Right.end())
    {
    }

    inline AListModel(p&& _Right)
    : p(_Right)
            {
            }

    template <class Iterator>
    inline AListModel(Iterator first, Iterator end): p(first, end) {}
    
    void push_back(const StoredType& data) {
        p::push_back(data);
        emit this->dataInserted(AModelRange{AModelIndex(p::size()-1), AModelIndex(p::size()), this});
    }

    AListModel& operator<<(const StoredType& data) {
        push_back(data);
        return *this;
    }

    size_t listSize() override {
        return p::size();
    }

    StoredType listItemAt(const AModelIndex& index) override {
        return p::at(index.getRow());
    }

};