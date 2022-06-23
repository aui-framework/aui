AUI uses GTest as the tests framework and provides a simple way to use it in your application.
                
In order to enable tests for your project, call `aui_enable_tests()` in the end of your `CMakeLists.txt` and create
`tests/` dir along with `src/` and `CMakeLists.txt`. Place your test suites inside the `tests/` directory.

# Example

Suppose we want to test the `sum` function of the following program:

```cpp
#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>
#include "main.h"
 
static constexpr auto LOG_TAG = "MyApp";

int sum(int a, int b) {
  return a + b;
}

AUI_ENTRY {
    ALogger::info(LOG_TAG) << "2 + 2 = " << sum(2, 2);
    return 0;
}
```

Possible output:

```
[14:57:02][UI Thread][MyApp][INFO]: 2 + 2 = 4
```

## Project structure

To write a test, enable tests for your projects first. Add `aui_enable_tests()` to the end of your `CMakeLists.txt`:

```cmake
# Standard routine
cmake_minimum_required(VERSION 3.16)
project(project_template)
 
# Use AUI.Boot
file(
    DOWNLOAD 
    https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
 
# link AUI
auib_import(
    AUI https://github.com/aui-framework/aui 
    COMPONENTS core)
 
 
# Create the executable. This function automatically links all sources from the src/ folder, creates CMake target and
# places the resulting executable to bin/ folder.
aui_executable(project_template)
 
# Link required libs
target_link_libraries(project_template PRIVATE aui::core)

aui_enable_tests() # <----------------------------------------------
```

and create the `tests/` dir along with `CMakeLists.txt` and `src/`. Inside `tests/`, create `MyTest.cpp`:

```
CMakeLists.txt
src
├── main.cpp
└── main.h
tests
└── MyTest.cpp

```

Note the `main.h` file. Since we would like to access the `sum` function of `main.cpp`, we should put `sum`
declaration to the `main.h` file:

```cpp
#pragma once

int sum(int a, int b);
```

Put the basic test case to `MyTest.cpp`:

```cpp
#include <gtest/gtest.h>
#include "main.h"

TEST(MyTest, Sum) {
  ASSERT_EQ(sum(2, 2), 4);
}
```

Build `Tests` and run `bin/Tests`. Possible output:

```
[==========] Running 1 test from 1 test suite.
[----------] Global test environment set-up.
[----------] 1 test from MyTest
[ RUN      ] MyTest.Sum
[       OK ] MyTest.Sum (0 ms)
[----------] 1 test from MyTest (0 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test suite ran. (0 ms total)
[  PASSED  ] 1 test.
```

AUI does the following fundamental things for you:

1. Collect all tests from your `tests/` dir
2. Provide an entry point for the `Tests` program (testing::InitGoogleTest)
3. Mirror all linked libraries and compile options to the `Tests` program
4. If you are testing a shared library, the shared library is linked to the `Tests` program
5. If you are testing an executable, the executable's sources are automatically compiled with the `Tests` program and
   the `include` scope of the executable is copied to the `Tests` program (this is why we have easily accessed the
   `main.h` header)