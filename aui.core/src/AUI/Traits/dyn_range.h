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
 * @details
 * `aui::dyn_range` follows the same principle as `std::function` for functors.
 */
template<typename T>
struct dyn_range {
    struct iterator {
        using value_type = T;
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;

        template<typename Iterator>
        iterator(Iterator&& it): mImpl(rttify(std::forward<Iterator>(it))) {

        }
        iterator(iterator&& rhs) noexcept = default;
        iterator& operator=(iterator&& rhs) noexcept = default;

        iterator() = default;

        iterator(const iterator& rhs) {
            operator=(rhs);
        }
        iterator& operator=(const iterator& rhs) {
            if (this == &rhs) {
                return *this;
            }
            mImpl = rhs.mImpl->clone();
            return *this;
        }

        iterator& operator++() {
            mImpl->next();
            return *this;
        }
        iterator& operator++(int) {
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

    template<typename Iterator>
    std::unique_ptr<iface> rttify(Iterator&& it) {
        struct rttified: iface {
            rttified(std::decay_t<Iterator> it): it(std::move(it)) {}
            ~rttified() = default;

            void next() override {
                // ++it;
            }

            T value() override {
                // return *it;
                return {};
            }

            std::unique_ptr<iface> clone() override {
                return std::make_unique<rttified>(it);
            }

            bool isEquals(iface* rhs) override {
                if (auto t = dynamic_cast<rttified*>(rhs)) {
                    // return it == t->it;
                }
                return false;
            }

        private:
            std::decay_t<Iterator> it;
        };
        return std::make_unique<rttified>(it);
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
            rttified(Rng f): rng(std::move(f)) {}
            ~rttified() = default;

            iterator begin() override {
                return ranges::begin(rng);

            }

            iterator end() override {
                return ranges::end(rng);
            }

            std::unique_ptr<iface> clone() override {
                return std::make_unique<rttified>(rng);
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

