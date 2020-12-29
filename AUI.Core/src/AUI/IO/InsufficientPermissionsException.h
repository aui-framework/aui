//
// Created by alex2 on 31.10.2020.
//

#pragma once

#include "IOException.h"

class InsufficientPermissionsException: public IOException {
public:
    InsufficientPermissionsException();

    InsufficientPermissionsException(const AString& message);
};


