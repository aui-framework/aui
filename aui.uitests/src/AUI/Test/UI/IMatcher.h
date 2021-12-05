#pragma once

#include <AUI/Common/SharedPtr.h>
#include <AUI/View/AView.h>


class IMatcher {
public:
    virtual ~IMatcher() = default;
    virtual bool matches(const _<AView>& view) = 0;
};


