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

//
// Created by alex2 on 07.11.2020.
//


#include <gtest/gtest.h>
#include <AUI/IO/APath.h>
#include <AUI/Common/AException.h>
#include <AUI/i18n/ALanguageCode.h>

TEST(I18n, LanguageCode) {
    ASSERT_THROW(ALanguageCode("en-UdS"), AException);
    ASSERT_THROW(ALanguageCode("end-US"), AException);
    ASSERT_THROW(ALanguageCode(""), AException);
    ASSERT_THROW(ALanguageCode("fweo,fwoepmo"), AException);
    ASSERT_THROW(ALanguageCode("fweo,-"), AException);
    ASSERT_THROW(ALanguageCode("-afs"), AException);

    ASSERT_NO_THROW(ALanguageCode("en_US"));
    ASSERT_NO_THROW(ALanguageCode("en-us"));
    ASSERT_NO_THROW(ALanguageCode("en-US"));

    ALanguageCode c("ru-UA");

    ASSERT_EQ(c.toString(), "ru-UA");
    ASSERT_EQ(c.getGroup(), "ru");
    ASSERT_EQ(c.getSubGroup(), "UA");
}
