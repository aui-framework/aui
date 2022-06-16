//
// Created by Alex2772 on 11/19/2021.
//

#pragma once


#include <AUI/Reflect/AEnumerate.h>

AUI_ENUM_FLAG(Repeat) {
        NONE = 0,
        X_Y = 0b11,
        X = 0b01,
        Y = 0b10,
};

AUI_ENUM_VALUES(Repeat, Repeat::NONE, Repeat::X, Repeat::Y, Repeat::X_Y)