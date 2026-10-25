#include <AUI/Platform/Entry.h>

#include "Cli/Cli.h"
#include "Gui/MainWindow.h"

AUI_ENTRY {
    if (auto exitCode = aui_remote::cli::tryRun(args)) {
        return *exitCode;
    }
    _new<MainWindow>()->show();
    return 0;
}
