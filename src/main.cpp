#include <AUI/Platform/Entry.h>

#include "cli/Cli.h"
#include "gui/MainWindow.h"

AUI_ENTRY {
    if (auto exitCode = aui_remote::cli::tryRun(args)) {
        return *exitCode;
    }
    _new<MainWindow>()->show();
    return 0;
}
