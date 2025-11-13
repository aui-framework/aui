/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Common/AException.h>
#include <range/v3/range/concepts.hpp>



namespace aui {
template<typename Rng, typename T>
concept range_consisting_of = requires(Rng&& rng) {
    { rng } -> ranges::range;
    { std::move(*::ranges::begin(rng)) } -> aui::convertible_to<T>;
};
}

namespace aui {

struct dyn_range_capabilities {
    bool implementsOperatorMinusMinus;
};

/**
 * @brief RTTI-wrapped range.
 * @ingroup core
 * @tparam T element type
 * @details
 * <!-- aui:experimental -->
 * `aui::any_range_view` is a dynamic range class that mimics the behavior of C++20 ranges/range-v3 using type-erased
 * interfaces. It allows for the creation of runtime-checked, polymorphic ranges with input iterators.
 *
 * Alternative implementation of `ranges::views::any_range_view`.
 *
 * The general idea is to preserve lazy nature of C++20 ranges/range-v3 and flexibility between compilation modules.
 *
 * Keep in mind that type erasure can lead to performance overhead due to dynamic dispatch.
 *
 * `aui::any_range_view` initialized with an lvalue reference will contain a reference to the container; thus the container
 * can be modified.
 *
 * <!-- aui:snippet aui.core/tests/IteratorsTest.cpp DynRange4 -->
 *
 * `aui::any_range_view` initialized with an rvalue reference will move the container into itself; thus it acquires
 * ownership.
 *
 * <!-- aui:snippet aui.core/tests/IteratorsTest.cpp DynRange5 -->
 *
 * Using `aui::any_range_view::iterator` acquired before modification of the referenced container may lead to undefined
 * behaviour; it all depends on the referenced container.
 *
 * `aui::any_range_view` follows the same principle as `std::function` for functors.
 */
template<typename T>
struct any_range_view {
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

        iterator& operator--() {
            mImpl->prev();
            return *this;
        }

        iterator& operator--(int) { // not accurate
            mImpl->prev();
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
            virtual void prev() = 0;
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

            void prev() override {
                if constexpr (requires { --it; }) {
                    --it;
                } else {
                    throw AException("unimplemented");
                }
            }

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

    template<ranges::input_range Rng>
    any_range_view(Rng&& rng): mImpl(rttify(std::forward<Rng>(rng))) {

    }

    any_range_view(): mImpl(nullptr) {}

    any_range_view(any_range_view&& rhs) noexcept = default;
    any_range_view& operator=(any_range_view&& rhs) noexcept = default;

    any_range_view(any_range_view& rhs) {
        operator=(rhs);
    }
    any_range_view(const any_range_view& rhs) {
        operator=(rhs);
    }

    any_range_view& operator=(any_range_view& rhs) {
        return operator=(const_cast<const any_range_view&>(rhs));
    }

    any_range_view& operator=(const any_range_view& rhs) {
        if (this == &rhs) {
            return *this;
        }
        mImpl = rhs.mImpl->clone();
        return *this;
    }

    /**
     * @return polymorphic begin iterator.
     */
    iterator begin() const {
        return mImpl->begin();
    }

    /**
     * @return polymorphic end iterator.
     */
    iterator end() const {
        return mImpl->end();
    }

    dyn_range_capabilities capabilities() const {
        if (!mImpl) {
            return {};
        }
        return mImpl->capabilities();
    }

private:
    struct iface {
        virtual ~iface() = default;

        virtual iterator begin() = 0;
        virtual iterator end() = 0;
        virtual std::unique_ptr<iface> clone() = 0;
        virtual dyn_range_capabilities capabilities() = 0;
    };

    std::unique_ptr<iface> mImpl;

    template<aui::range_consisting_of<T> Rng>
    std::unique_ptr<iface> rttify(Rng&& rng) {
        static_assert(!requires() { rng.mImpl; }, "rttifying itself?");

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

            dyn_range_capabilities capabilities() override {
                return {
                    .implementsOperatorMinusMinus = requires() { --++rng.begin(); },
                };
            }

        private:
            Rng rng;
        };
        return std::make_unique<rttified>(std::forward<Rng>(rng));
    }
};

static_assert(ranges::input_iterator<any_range_view<int>::iterator>);
static_assert(ranges::sentinel_for<any_range_view<int>::iterator, any_range_view<int>::iterator>);
static_assert(ranges::range<any_range_view<int>>);

}

