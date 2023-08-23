#pragma once

#include <cassert>
#include "winnt.h"

struct AssertOkHelper {
    void operator+(HRESULT r) const {
        assert(!FAILED(r));
    }
};

#define ASSERT_OK AssertOkHelper{} +
