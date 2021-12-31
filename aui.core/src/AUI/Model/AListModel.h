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
#include <AUI/Traits/strings.h>

namespace aui::detail {

}

template <typename StoredType>
class AListModel: public AObject,
                  public IMutableListModel<StoredType> {
private:
    AVector<StoredType> mVector;

    using self = AListModel<StoredType>;

public:
    using iterator = typename decltype(mVector)::iterator;
    using const_iterator = typename decltype(mVector)::const_iterator;
    using size_type = typename decltype(mVector)::size_type;
    using value_type = typename decltype(mVector)::value_type;

    AListModel() = default;
    AListModel(const self& s): mVector(s.mVector) {}
    AListModel(self&& s): mVector(std::move(s.mVector)) {}


    [[nodiscard]]
    bool empty() const noexcept {
        return mVector.empty();
    }
    void reserve(size_t s) noexcept {
        mVector.reserve(s);
    }
    void resize(size_t s) noexcept {
        mVector.resize(s);
    }

    iterator erase(iterator begin) noexcept  {
        return this->erase(begin, begin + 1);
    }
    iterator erase(iterator begin, iterator end) noexcept {
        AModelRange range{AModelIndex{size_t(begin - mVector.begin())},
                          AModelIndex{size_t(end   - mVector.begin())},
                          this};
        auto it = mVector.erase(begin, end);
        emit this->dataRemoved(range);

        return it;
    }


    void push_back(const StoredType& data) noexcept {
        mVector.push_back(data);
        emit this->dataInserted(AModelRange{AModelIndex(mVector.size() - 1),
                                            AModelIndex(mVector.size()    ),
                                            this});
    }


    void push_back(StoredType&& data) noexcept {
        mVector.push_back(std::forward<StoredType>(data));
        emit this->dataInserted(AModelRange{AModelIndex(mVector.size() - 1),
                                            AModelIndex(mVector.size()    ),
                                            this});
    }


    void pop_back() noexcept {
        mVector.pop_back();
        emit this->dataRemoved(AModelRange{AModelIndex(mVector.size()    ),
                                           AModelIndex(mVector.size() + 1),
                                           this});
    }

    AListModel& operator<<(const StoredType& data) noexcept {
        push_back(data);
        return *this;
    }
    AListModel& operator<<(StoredType&& data) noexcept {
        push_back(std::forward<StoredType>(data));
        return *this;
    }

    size_t listSize() override {
        return mVector.size();
    }

    StoredType listItemAt(const AModelIndex& index) override {
        return mVector.at(index.getRow());
    }
    void invalidate(size_t index) {
        emit this->dataChanged(AModelRange{AModelIndex(index), AModelIndex(index + 1u), this});
    }

    void clear() noexcept {
        erase(mVector.begin(), mVector.end());
    }

    void removeItems(const AModelRange<StoredType>& items) override {
        erase(mVector.begin() + items.begin().getIndex().getRow(), mVector.end() + items.begin().getIndex().getRow());
    }

    void removeItem(const AModelIndex& item) override {
        erase(mVector.begin() + item.getRow());
    }

    [[nodiscard]]
    size_t size() const noexcept {
        return mVector.size();
    }

    /**
     * Fetches element by index.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd><code>AException</code> thrown if out of bounds.</dd>
     * </dl>
     * <dl>
     *   <dt><b>Performance note</b></dt>
     *   <dd>if guaranteed <code>index</code> is not out of bounds, use the <code>operator[]</code> function instead.</dd>
     * </dl>
     */
    const StoredType& at(size_t index) const {
        if (index >= size()) throw AException("index of bounds: {} (size {})"_format(index, size()));
        return *(mVector.begin() + index);
    }

    /**
     * Fetches element by index.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd><code>AException</code> thrown if out of bounds.</dd>
     * </dl>
     * <dl>
     *   <dt><b>Performance note</b></dt>
     *   <dd>if guaranteed <code>index</code> is not out of bounds, use the <code>operator[]</code> function instead.</dd>
     * </dl>
     * <dl>
     *   <dt><b>Data model note</b></dt>
     *   <dd>
     *      A mutable reference returned by this method. If you want to change data, you should report changes by
     *      <a href="#invalidate">invalidate</a> function.
     *   </dd>
     * </dl>
     */
    StoredType& at(size_t index) {
        if (index >= size()) throw AException("index of bounds: {} (size {})"_format(index, size()));
        return *(mVector.begin() + index);
    }

    /**
     * Fetches element by index.
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd><code>index</code> points to the existing element.</dd>
     * </dl>
     */
    const StoredType& operator[](size_t index) const {
        assert(("index out of bounds" && size() <= index));
        return *(mVector.begin() + index);
    }

    /**
     * Removes element at the specified index.
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd><code>index</code> points to the existing element.</dd>
     * </dl>
     * @param index index of the element.
     */
    void removeAt(size_t index) noexcept
    {
        aui::container::remove_at(*this, index);
        emit this->dataRemoved(AModelRange{AModelIndex(index    ),
                                           AModelIndex(index + 1),
                                           this});
    }

    const_iterator begin() const {
        return mVector.begin();
    }
    const_iterator end() const {
        return mVector.end();
    }

    iterator begin() {
        return mVector.begin();
    }
    iterator end() {
        return mVector.end();
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
        auto list = _new<AListModel<StoredType>>();
        list->reserve(t.size());
        for (auto& item : t) {
            list->push_back(item);
        }
        return list;
    }
};