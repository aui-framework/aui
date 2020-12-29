//
// Created by alex2 on 05.12.2020.
//

#pragma once


#include <AUI/View/AViewContainer.h>
#include <AUI/Platform/CustomCaptionWindowImplWin32.h>

class FakeWindow: public AViewContainer, public CustomCaptionWindowImplWin32 {
public:
    FakeWindow();
    virtual ~FakeWindow() = default;


protected:
    bool isCustomCaptionMaximized() override;
};


