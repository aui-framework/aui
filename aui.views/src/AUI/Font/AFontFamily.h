// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once


#include <AUI/Common/AMap.h>

class AFont;

class AFontFamily {
public:
    enum Weight {
        W100,
        W200,
        W300,
        W400,
        W500,
        W600,
        W700,
        W800,
        W900,
        NORMAL = W400,
        BOLD   = W700,
    };


private:
    struct {
        _<AFont> nonItalic;
        _<AFont> italic;
    } mFonts[9]; // weights 100-900;


    template<bool italic>
    _<AFont> getEitherWeight() {
        for (auto& e : {
            W400,
            W500,
            W300,
            W600,
            W200,
            W700,
            W100,
            W800,
            W900
        }) {
            if (auto& f = get(NORMAL, italic)) {
                return f;
            }
        }
        return nullptr;
    }
    template<bool italicPrefer>
    _<AFont> getEitherWeightItalicPreferred() {
        if (auto f = getEitherWeight<italicPrefer>()) {
            return f;
        }
        return getEitherWeight<!italicPrefer>();
    }

public:
    _<AFont>& get(Weight weight, bool isItalic) {
        auto& r = mFonts[weight];
        return isItalic ? r.italic : r.nonItalic;
    }

    /**
     * @param italicPreferred when <code>true</code>, prefers italic fonts over non-italic.
     * @return instance of <code>AFont</code> of this font family with weight of 400. If font family does not have
     *         font with weight of 400, the nearest to this weight existing font is returned instead.
     */
    _<AFont> getEither(bool italicPreferred = false) {
        if (italicPreferred) {
            return getEitherWeightItalicPreferred<true>();
        }
        return getEitherWeightItalicPreferred<false>();
    }
};


