#pragma once


#include <AUI/Platform/AWindow.h>
#include <AUI/View/ALabel.h>

class TestWindow: public AWindow {
private:
    _<ALabel> mHelloLabel;

public:
    TestWindow();
    static void make();
};


