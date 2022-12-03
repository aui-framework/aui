// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by alex2 on 07.11.2020.
//


#include <gtest/gtest.h>
#include <AUI/IO/APath.h>
#include <AUI/Common/AException.h>
#include <AUI/i18n/ALanguageCode.h>

TEST(I18n, LanguageCode) {
        ASSERT_THROW(ALanguageCode("en_US"), AException);
        ASSERT_THROW(ALanguageCode("en-us"), AException);
        ASSERT_THROW(ALanguageCode("en-UdS"), AException);
        ASSERT_THROW(ALanguageCode("end-US"), AException);
        ASSERT_THROW(ALanguageCode(""), AException);
        ASSERT_THROW(ALanguageCode("fweo,fwoepmo"), AException);
        ASSERT_THROW(ALanguageCode("fweo,-"), AException);
        ASSERT_THROW(ALanguageCode("-afs"), AException);

        ASSERT_NO_THROW(ALanguageCode("en-US"));

        ALanguageCode c("ru-UA");

        ASSERT_EQ(c.toString(), "ru-UA");
        ASSERT_EQ(c.getGroup(), "ru");
        ASSERT_EQ(c.getSubGroup(), "UA");
}
