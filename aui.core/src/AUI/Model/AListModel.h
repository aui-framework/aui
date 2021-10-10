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

#include <AUI/Common/AVector.h>
#include <AUI/Common/ASignal.h>
#include "AModelIndex.h"
#include "IMutableListModel.h"

namespace aui::detail {

    template <typename StoredType>
    class AListModel: public std::vector<StoredType>, public AObject, public IMutableListModel<StoredType> {
    private:
        using p = std::vector<StoredType>;
        using Iterator = typename p::iterator;

    public:
#ifdef _MSC_VER
        using p::vector;
#else
        using typename p::vector;
#endif


        Iterator erase(Iterator begin) {
            return this->erase(begin, begin + 1);
        }
        Iterator erase(Iterator begin, Iterator end) {
            AModelRange range{AModelIndex{size_t(begin - this->begin())},
                              AModelIndex{size_t(end   - this->begin())},
                              this};
            auto it = p::erase(begin, end);
            emit this->dataRemoved(range);

            return it;
        }


        void push_back(const StoredType& data) {
            p::push_back(data);
            emit this->dataInserted(AModelRange{AModelIndex(p::size() - 1),
                                                AModelIndex(p::size()    ),
                                                this});
        }


        void pop_back() {
            p::pop_back();
            emit this->dataRemoved(AModelRange{AModelIndex(p::size()    ),
                                               AModelIndex(p::size() + 1),
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

        void clear() {
            erase(p::begin(), p::end());
        }

        void removeItems(const AModelRange<StoredType>& items) override {
            erase(p::begin() + items.begin().getIndex().getRow(), p::end() + items.begin().getIndex().getRow());
        }

        void removeItem(const AModelIndex& item) override {
            erase(p::begin() + item.getRow());
        }


        /**
         * Create AListModel from initializer list. Applicable for initializing AListModel<AString> from
         * const char* initializer list.
         *
         * @tparam V type that converts to T
         * @return a new AListModel
         */
        template<typename V>
        static _<AListModel<StoredType>> make(const std::initializer_list<V>& t) {
            if constexpr(std::is_same_v<V, StoredType>) {
                return _new<AListModel<AString>>(t.begin(), t.end());
            } else {
                auto model = _new<AListModel<AString>>(t.begin(), t.end());
                model->reserve(t.end() - t.begin());
                for (auto& element : t) {
                    model->push_back(StoredType(element));
                }
                return model;
            }
        }
    };
}

template <typename StoredType>
using AListModel = SequenceContainerExtensions<aui::detail::AListModel<StoredType>>;