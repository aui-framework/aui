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

#include <AUI/Common/AException.h>
#include <range/v3/range_concepts.hpp>


namespace aui {

/**
 * @brief RTTI-wrapped range.
 * @ingroup core
 * @tparam T element type
 * @details
 * @experimental
 * `aui::dyn_range` is a dynamic range class that mimics the behavior of C++20 ranges/range-v3 using type-erased
 * interfaces. It allows for the creation of runtime-checked, polymorphic ranges with input iterators.
 *
 * The general idea is to preserve lazy nature of C++20 ranges/range-v3 and flexibility between compilation modules.
 *
 * Keep in mind that type erasure can lead to performance overhead due to dynamic dispatch.
 *
 * `aui::dyn_range` initialized with an lvalue reference will contain a reference to the container; thus the container
 * can be modified.
 *
 * @snippet aui.core/tests/IteratorsTest.cpp DynRange4
 *
 * `aui::dyn_range` initialized with an rvalue reference will move the container into itself; thus it acquires
 * ownership.
 *
 * @snippet aui.core/tests/IteratorsTest.cpp DynRange5
 *
 * Using `aui::dyn_range::iterator` acquired before modification of the referenced container may lead to undefined
 * behaviour; it all depends on the referenced container.
 *
 * `aui::dyn_range` follows the same principle as `std::function` for functors.
 */
template<typename T>
struct dyn_range {
    struct iterator {
        using value_type = T;
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;

        template<ranges::input_range Rng, typename Iterator>
        iterator(Rng&& rng, Iterator&& it): mImpl(rttify(std::forward<Rng>(rng), std::forward<Iterator>(it))) {

        }
        iterator(iterator&& rhs) noexcept = default;
        iterator& operator=(iterator&& rhs) noexcept = default;

        iterator() = default;

        iterator(iterator& rhs) {
            operator=(rhs);
        }
        iterator(const iterator& rhs) {
            operator=(rhs);
        }
        iterator& operator=(const iterator& rhs) {
            if (this == &rhs) {
                return *this;
            }
            if (rhs.mImpl) {
                mImpl = rhs.mImpl->clone();
            }
            return *this;
        }

        iterator& operator++() {
            mImpl->next();
            return *this;
        }
        iterator& operator++(int) { // not accurate
            mImpl->next();
            return *this;
        }

        T operator*() const {
            return mImpl->value();
        }

        bool operator==(const iterator& rhs) const {
            return mImpl->isEquals(rhs.mImpl.get());
        }

        bool operator!=(const iterator& rhs) const {
            return !mImpl->isEquals(rhs.mImpl.get());
        }

    private:
        struct iface {
            virtual ~iface() = default;
            virtual void next() = 0;
            virtual T value() = 0;
            virtual std::unique_ptr<iface> clone() = 0;
            virtual bool isEquals(iface* rhs) = 0;
        };
        std::unique_ptr<iface> mImpl;

    template<ranges::input_range Rng, typename Iterator>
    std::unique_ptr<iface> rttify(Rng&& rng, Iterator&& it) {
        struct rttified: iface {
            rttified(std::remove_reference_t<Rng>& rng, std::decay_t<Iterator> it): rng(rng), it(std::move(it)) {}
            ~rttified() = default;

            void next() override {
                 ++it;
            }

            T value() override {
                if constexpr (std::is_copy_constructible_v<T>) {
                    return *it;
                }
                throw AException("can't make a copy");
            }

            std::unique_ptr<iface> clone() override {
                return std::make_unique<rttified>(rng, it);
            }

            bool isEquals(iface* rhs) override {
                if (rhs == nullptr) {
                    // end sentinel; ask rng for end by ourselves
                    return it == ranges::end(rng);
                }
                if (auto t = dynamic_cast<rttified*>(rhs)) {
                     return it == t->it;
                }
                return false;
            }

        private:
            std::decay_t<Iterator> it;
            std::remove_reference_t<Rng>& rng;
        };

        return std::make_unique<rttified>(rng, it);
    }
    };

    template<ranges::range Rng>
    dyn_range(Rng&& rng): mImpl(rttify(std::forward<Rng>(rng))) {

    }

    dyn_range(): mImpl(nullptr) {}

    dyn_range(dyn_range&& rhs) noexcept = default;
    dyn_range& operator=(dyn_range&& rhs) noexcept = default;
    dyn_range(const dyn_range& rhs) {
        operator=(rhs);
    }
    dyn_range& operator=(const dyn_range& rhs) {
        if (this == &rhs) {
            return *this;
        }
        mImpl = rhs.mImpl->clone();
        return *this;
    }

    iterator begin() const {
        return mImpl->begin();
    }

    iterator end() const {
        return mImpl->end();
    }

private:
    struct iface {
        virtual ~iface() = default;

        virtual iterator begin() = 0;
        virtual iterator end() = 0;
        virtual std::unique_ptr<iface> clone() = 0;

    };

    std::unique_ptr<iface> mImpl;

    template<ranges::range Rng>
    std::unique_ptr<iface> rttify(Rng&& rng) {
        struct rttified: iface {
        public:
            rttified(Rng f): rng(std::forward<Rng>(f)) {}
            ~rttified() = default;

            iterator begin() override {
                return { rng, ranges::begin(rng) };

            }

            iterator end() override {
                if constexpr (std::is_same_v<decltype(ranges::begin(rng)), decltype(ranges::end(rng))>) {
                    return { rng, ranges::end(rng) };
                } else {
                    // sentinel?
                    return {};
                }
            }

            std::unique_ptr<iface> clone() override {
                if constexpr (std::is_copy_constructible_v<T>) {
                    return std::make_unique<rttified>(rng);
                }
                throw AException("can't make a copy of this range");
            }

        private:
            Rng rng;
        };
        return std::make_unique<rttified>(std::forward<Rng>(rng));
    }
};

static_assert(ranges::input_iterator<dyn_range<int>::iterator>);
static_assert(ranges::sentinel_for<dyn_range<int>::iterator, dyn_range<int>::iterator>);
static_assert(ranges::range<dyn_range<int>>);

}

