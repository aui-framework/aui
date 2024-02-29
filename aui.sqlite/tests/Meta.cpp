// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

//
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Data/AMeta.h>


TEST(Meta, Meta) {
    Autumn::put(ASqlDatabase::connect("sqlite", ":memory:"));
    AMeta::set("kek", 4);
    AMeta::set("lol", "azaza");
    ASSERT_EQ(AMeta::get("kek"), 4);
    ASSERT_EQ(AMeta::get("lol"), "azaza");
    AMeta::set("kek", "four");
    AMeta::set("lol", 42.0);
    ASSERT_EQ(AMeta::get("kek"), "four");
    ASSERT_EQ(AMeta::get("lol"), 42.0);
}

