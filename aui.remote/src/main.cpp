#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/Entry.h>

#include "cli/Cli.h"
#include "ui/window/MainWindow.h"

#if AUI_REMOTE_SELF_DEBUG
#include <AUI/Remote/ARemoteDebug.h>
#endif

AUI_ENTRY {
    if (auto exitCode = aui_remote::cli::tryRun(args)) {
        return *exitCode;
    }
#if AUI_REMOTE_SELF_DEBUG
    // the GUI only: a CLI process would leave a discovery file behind on every command
    ARemoteDebug::start(ARemoteDebug::Options::fromEnvironment());
#endif
    auto window = _new<AWindow>(AWindow::Init {
      .title = "AUI Remote",
      .width = 900_dp,
      .height = 600_dp,
      .decoration = AWindow::Init::DecorationExtended {},
    });
    window->setContents(mainWindowContents());
    window->show();
    return 0;
}
