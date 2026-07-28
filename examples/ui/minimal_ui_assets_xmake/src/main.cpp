#include <AUI/Platform/Entry.h>
#include "MainWindow.h"

AUI_ENTRY {
    _new<MainWindow>()->show();
    return 0;
};
