#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>
int someFunctionFromExecutable(int a, int b);

int someFunctionFromExecutable(int a, int b) {
    return a + b;
}

AUI_ENTRY {
    ALogger::info("Hello world!");
    return 0;
}