// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "AForEachUILazy.h"

namespace {
template <typename T>
class SlidingWindowListModel final : public IListModel<T> {
public:
    explicit SlidingWindowListModel(_<IListModel<T>> wrapped) : mWrapped(std::move(wrapped)) {
        for (auto f : { &IListModel<T>::dataInserted, &IListModel<T>::dataChanged, &IListModel<T>::dataRemoved }) {
            AObject::connect(std::invoke(f, *mWrapped), this, [this, f](const AListModelRange<T>& range) {
                if (auto rng = wrapped2self(range); !rng.empty()) {
                    emit std::invoke(f, this)(range);
                }
            });
        }
    }
    ~SlidingWindowListModel() override = default;
    size_t listSize() override {
        const auto wrappedSize = mWrapped->listSize();
        if (wrappedSize <= mOffset) {
            return 0;
        }
        return glm::min(wrappedSize - mOffset, mWindowSize);
    }
    T listItemAt(const AListModelIndex& index) override { return mWrapped->listItemAt(index.getRow() - mOffset); }

private:
    _<IListModel<T>> mWrapped;
    std::size_t mOffset = 0;
    std::size_t mWindowSize = 20;

    AListModelIndex wrapped2self(const AListModelIndex& wrapped) {
        return glm::clamp(wrapped.getRow() - mOffset, std::size_t(0), mWindowSize - 1);
    }

    AListModelRange<T> wrapped2self(const AListModelRange<T>& wrapped) {
        return this->range(wrapped2self(wrapped.getBegin()), wrapped2self(wrapped.getEnd()));
    }
};
}   // namespace

void AForEachUILazy::setModelImpl(_<IListModel<_<AView>>> model) {
    super::setModelImpl(_new<SlidingWindowListModel<_<AView>>>(std::move(model)));
}
