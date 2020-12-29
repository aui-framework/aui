#pragma once


#include <AUI/Platform/AWindow.h>
#include "FractalView.h"

class JumpToCoordsWindow: public AWindow {
public:
    explicit JumpToCoordsWindow(_<FractalView> fractalView, AWindow* parent);
};

