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

#include <tuple>
#include <variant>
#include <cstdint>
#include <iterator>
#include <ostream>
#include "parameter_pack.h"
#include "macros.h"
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
public:
    struct iface {
    public:
        virtual ~iface() = default;

        virtual int begin() = 0;
        virtual int end() = 0;
    };

    template<ranges::range Rng>
    dyn_range(Rng&& rng): mInterface(rttify(std::forward<Rng>(rng))) {

    }

private:
    std::unique_ptr<iface> mInterface;

    template<ranges::range Rng>
    std::unique_ptr<iface> rttify(Rng&& rng) {
        struct rttified: iface, Rng {
        public:
            rttified(Rng&& f): Rng(std::forward<Rng>(f)) {}
            ~rttified() = default;

            int begin() override {

            }

            int end() override {

            }
        };

        return std::make_unique<rttified>(std::forward<Rng>(rng));
    }
};

}

