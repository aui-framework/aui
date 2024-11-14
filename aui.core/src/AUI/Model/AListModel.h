/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/Common/ASignal.h>
#include "AListModelIndex.h"
#include "IMutableListModel.h"
#include <AUI/Traits/strings.h>

namespace aui::detail {

}

template <typename StoredType>
class AListModel: public IRemovableListModel<StoredType>,
                  public IValueMutableListModel<StoredType> {
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
    AListModel(self&& s) noexcept: mVector(std::move(s.mVector)) {}
    explicit AListModel(AVector<StoredType>&& vector) noexcept: mVector(std::move(vector)) {}

    AListModel& operator=(AVector<StoredType>&& rhs) noexcept {
        clear();
        mVector = std::move(rhs);
        emit this->dataInserted(this->range(AListModelIndex(0),
                                            AListModelIndex(mVector.size())));
        return *this;
    }

    void setItem(const AListModelIndex& item, const StoredType& value) override {
        mVector[item.getRow()] = value;
    }

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
        auto range = this->range(AListModelIndex{size_t(begin - mVector.begin())},
                                 AListModelIndex{size_t(end   - mVector.begin())});
        auto it = mVector.erase(begin, end);
        emit this->dataRemoved(range);

        return it;
    }


    void push_back(const StoredType& data) noexcept {
        mVector.push_back(data);
        emit this->dataInserted(this->range(AListModelIndex(mVector.size() - 1),
                                            AListModelIndex(mVector.size()    )));
    }


    void push_back(StoredType&& data) noexcept {
        mVector.push_back(std::forward<StoredType>(data));
        emit this->dataInserted(this->range(AListModelIndex(mVector.size() - 1),
                                            AListModelIndex(mVector.size()    )));
    }


    void pop_back() noexcept {
        mVector.pop_back();
        emit this->dataRemoved(this->range(AListModelIndex(mVector.size()    ),
                                           AListModelIndex(mVector.size() + 1)));
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

    StoredType listItemAt(const AListModelIndex& index) override {
        return mVector.at(index.getRow());
    }
    void invalidate(size_t index) {
        emit this->dataChanged(this->range(AListModelIndex(index), AListModelIndex(index + 1u)));
    }

    void invalidate(iterator index) {
        invalidate(std::distance(begin(), index));
    }

    void clear() noexcept {
        erase(mVector.begin(), mVector.end());
    }

    void removeItems(const AListModelRange<StoredType>& items) override {
        erase(mVector.begin() + items.begin().getIndex().getRow(), mVector.end() + items.begin().getIndex().getRow());
    }

    void removeItem(const AListModelIndex& item) override {
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
        AUI_ASSERTX(size() > index, "index out of bounds");
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

    /**
     * Create AListModel from initializer list. Applicable for initializing AListModel<AString> from
     * const char* initializer list.
     *
     * @tparam V type that converts to T
     * @return a new AListModel
     */
    template<typename V>
    static _<AListModel<StoredType>> fromVector(AVector<V> t) {
        auto list = _new<AListModel<StoredType>>();
        list->mVector = std::move(t);
        return list;
    }

    /**
     * Create AListModel from initializer list. Applicable for initializing AListModel<AString> from
     * const char* initializer list.
     *
     * @tparam V type that converts to T
     * @return a new AListModel
     */
    template<typename V>
    static _<AListModel<StoredType>> fromVector(std::vector<V> t) {
        auto list = _new<AListModel<StoredType>>();
        list->mVector = AVector<StoredType>(std::vector<StoredType>(std::move(t)));
        return list;
    }

    template<typename UnaryOperation>
    auto map(UnaryOperation&& transformer) {
        return mVector.map(std::forward<UnaryOperation>(transformer));
    }

    [[nodiscard]]
    const AVector<StoredType>& toVector() noexcept {
        return mVector;
    }
};
