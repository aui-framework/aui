#pragma once


#include "AViewContainer.h"
#include <AUI/Model/IListModel.h>
#include <functional>
#include <AUI/Util/ADataBinding.h>

template<typename T>
class AForEach: public AViewContainer {
public:
    using List = _<IListModel<T>>;
    using Factory = std::function<_<AView>(ADataBinding<T>&)>;
private:
    List mList;
    Factory mFactory;

public:
    AForEach(const List& list, const Factory& factory) : mFactory(factory) {
        setModel(list);
    }

};

