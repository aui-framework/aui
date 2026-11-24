#include <base.h>
#include <cstdio>

const char* mid_base_version();

int main() {
    std::printf("project=%s mid=%s\n", BASE_VERSION, mid_base_version());
}
