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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/Common/ASignal.h>
#include "AModelIndex.h"
#include "IListModel.h"

namespace aui::detail {

    template <typename StoredType>
    class AListModel: public std::vector<StoredType>, public AObject, public IListModel<StoredType> {
    private:
        using p = std::vector<StoredType>;
        using Iterator = typename p::iterator;

    public:
        using p::vector;


        Iterator erase(Iterator begin) {
            return this->erase(begin, begin + 1);
        }
        Iterator erase(Iterator begin, Iterator end) {
            emit this->dataRemoved(AModelRange{AModelIndex{size_t(begin - this->begin())},
                                               AModelIndex{size_t(end   - this->begin())},
                                               this});

            return p::erase(begin, end);
        }


        void push_back(const StoredType& data) {
            p::push_back(data);
            emit this->dataInserted(AModelRange{AModelIndex(p::size() - 1),
                                                AModelIndex(p::size()    ),
                                                this});
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
        void invalidate(size_t index) {
            emit this->dataChanged(AModelRange{AModelIndex(index), AModelIndex(index + 1u), this});
        }
    };
}

template <typename StoredType>
class AListModel: public SequenceContainerExtensions<aui::detail::AListModel<StoredType>> {
    using p = SequenceContainerExtensions<aui::detail::AListModel<StoredType>>;
    using Iterator = typename p::Iterator;
public:
    using p::SequenceContainerExtensions;


};