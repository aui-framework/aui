// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>

namespace aui {

namespace hash_detail {

template <std::size_t Bits>
struct hash_mix_impl;

// hash_mix for 64 bit size_t
//
// The general "xmxmx" form of state of the art 64 bit mixers originates
// from Murmur3 by Austin Appleby, which uses the following function as
// its "final mix":
//
//	k ^= k >> 33;
//	k *= 0xff51afd7ed558ccd;
//	k ^= k >> 33;
//	k *= 0xc4ceb9fe1a85ec53;
//	k ^= k >> 33;
//
// (https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp)
//
// It has subsequently been improved multiple times by different authors
// by changing the constants. The most well known improvement is the
// so-called "variant 13" function by David Stafford:
//
//	k ^= k >> 30;
//	k *= 0xbf58476d1ce4e5b9;
//	k ^= k >> 27;
//	k *= 0x94d049bb133111eb;
//	k ^= k >> 31;
//
// (https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html)
//
// This mixing function is used in the splitmix64 RNG:
// http://xorshift.di.unimi.it/splitmix64.c
//
// We use Jon Maiga's implementation from
// http://jonkagstrom.com/mx3/mx3_rev2.html
//
// 	x ^= x >> 32;
//	x *= 0xe9846af9b1a615d;
//	x ^= x >> 32;
//	x *= 0xe9846af9b1a615d;
//	x ^= x >> 28;
//
// An equally good alternative is Pelle Evensen's Moremur:
//
//	x ^= x >> 27;
//	x *= 0x3C79AC492BA7B653;
//	x ^= x >> 33;
//	x *= 0x1C69B3F74AC4AE35;
//	x ^= x >> 27;
//
// (https://mostlymangling.blogspot.com/2019/12/stronger-better-morer-moremur-better.html)

template <>
struct hash_mix_impl<64> {
    inline static std::uint64_t fn(std::uint64_t x) {
        std::uint64_t const m = (static_cast<std::uint64_t>(0xe9846af) << 32) + 0x9b1a615d;

        x ^= x >> 32;
        x *= m;
        x ^= x >> 32;
        x *= m;
        x ^= x >> 28;

        return x;
    }
};

// hash_mix for 32 bit size_t
//
// We use the "best xmxmx" implementation from
// https://github.com/skeeto/hash-prospector/issues/19

template <>
struct hash_mix_impl<32> {
    inline static std::uint32_t fn(std::uint32_t x) {
        std::uint32_t const m1 = 0x21f0aaad;
        std::uint32_t const m2 = 0x735a2d97;

        x ^= x >> 16;
        x *= m1;
        x ^= x >> 15;
        x *= m2;
        x ^= x >> 15;

        return x;
    }
};

inline std::size_t hash_mix(std::size_t v) { return hash_mix_impl<sizeof(std::size_t) * CHAR_BIT>::fn(v); }

}   // namespace hash_detail

//
// boost::hash_combine
//

template <class T>
inline void hash_combine(std::size_t& seed, T const& v) {
    seed = aui::hash_detail::hash_mix(seed + 0x9e3779b9 + std::hash<T>()(v));
}

namespace hash_detail {
template <class It>
inline std::size_t hash_range(std::size_t seed, It first, It last) {
    for (; first != last; ++first) {
        hash_combine<typename std::iterator_traits<It>::value_type>(seed, *first);
    }

    return seed;
}
}

//
// boost::hash_range
//

template <class It>
inline void hash_range(std::size_t& seed, It first, It last) {
    seed = hash_detail::hash_range(seed, first, last);
}

template <class It>
inline std::size_t hash_range(It first, It last) {
    std::size_t seed = 0;

    hash_range(seed, first, last);

    return seed;
}
}   // namespace aui
