# Advanced Universal Interface manual

## Building AUI

Before to get started, you have to compile AUI.

### Windows


1. Download the following dependencies:
    - for `AUI.Views`: `freetype2`. Note that you need only `AUI.Views` for UI applications
    - for `AUI.Crypt`: `OpenSSL`
    - for `AUI.Curl`: `libcurl`
2. Download and install [cmake-gui](https://cmake.org/download/)
3. For each dependency:
   1. Open project's dir as source dir in cmake-gui
   2. Specify build folder
   3. Configure
   4. Generate
   5. Open project (button in cmake-gui)
   6. Build the `INSTALL` project. It will build the library and install to your system
4. `git clone https://github.com/Alex2772/aui.git`
5. `git submodule update --init --recursive`
6. Repeat the third step but for AUI

### *nix
1. Download and install cmake and gcc
2. `git clone https://github.com/Alex2772/aui.git`
3. Download dependencies: `git submodule update --init --recursive`
4. Install dependencies: `sudo apt install pkg-config libglew-dev zlib1g-dev libssl-dev libcrypt-dev libcurl4-openssl-dev`
5. Create build folder and cd to it: `cd aui && mkdir build && cd build`
6. Run `cmake ..`
7. Run `make && sudo make install`


## Basic hello world

**Project:** `projects/basic_hello_world`

Once you have compiled AUI, you can use it!

Here's the example of basic hello work application, which every AUI project AUI starts with.

`CMakeLists.txt`

```cmake
# Standard routine
cmake_minimum_required(VERSION 3.16)
project(project_template)

# Link AUI
find_package(AUI REQUIRED)

# Create the executable. This function automatically links all sources from the src/ folder, creates CMake target and
# places the resulting executable to bin/ folder.
AUI_Executable(project_template)

# Link required libs
target_link_libraries(project_template PRIVATE AUI.Core)
# target_link_libraries(project_template PRIVATE AUI.Views)
```

`src/main.cpp`

```cpp
#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>

AUI_ENTRY {
    ALogger::info("Hello world!");
    return 0;
}
```

Building and running this project, you are getting sure that AUI built properly and you are ready to build AUI applications!

## Graphical hello world

Since AUI is graphical framework it allows to easily create windows, buttons, fields without any graphical UI toolkits.

Don't forget to link to `AUI.Views` in `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(graphical_example)

# Link AUI
find_package(AUI REQUIRED)

# Create the executable. This function automatically links all sources from the src/ folder, creates CMake target and
# places the resulting executable to bin/ folder.
AUI_Executable(graphical_example)

# Link required libs
target_link_libraries(graphical_example PRIVATE AUI.Core)
target_link_libraries(graphical_example PRIVATE AUI.Views)
```

The last line in CMake script links the AUI.Views module which holds all UI related functionality of the framework.



# Reference

## AMetric

Used to store dimensions in scalable units (dp, pt, etc...).

It's highly recommended to use only Density-independent Pixel unit (`_dp`) to make your application correctly handle
systems with high DPI. In AUI, all units are density independent except `_px`. The only purpose of the `_px` unit is to
define lines of exact one or two pixels wide.

### Initialization

Common usage:

```c++
AMetric a = 5_dp;
a.getValuePx() // 5 on 100% scale, 6 on 125% scale, etc
```

AMetric can be also initialized via value and unit:

```c++
AMetric a(5, T_DP);
```

AMetric can be also initialized with zero without unit specified (in this case, AMetric::getUnit will return T_PX):

```c++
AMetric zero1 = 0; // zero pixels
AMetric zero2 = {}; // also zero pixels
```

However, if you try to specify nonzero integer without unit, it will produce a runtime error:

```c++
AMetric a = 5; // runtime error
```



### Supported units

Here's table of currently supported units:

| Unit                       | Enum | Literal |
| -------------------------- | ---- | ------- |
| Density-independent Pixels | T_DP | _dp     |
| Typography point           | T_PT | _pt     |
| Pixels                     | T_PX | _px     |


## Assertions

The whole AUI framework's code filled with assertion checks so if you do something wrong the framework will tell you
about it. Also in AUI almost every assertion contains a quick tip how to solve the problem It is recommended to you to
do the same. For example:

```cpp
assert(("AAbstractThread::processMessages() should not be called from other thread",
        mId == std::this_thread::get_id()));
```

The code above ensures that the function was not called from some other thread. As you can see, the tooltip is produced
using the `operator,`:

```cpp
assert(("your message", expression))
```

Assertions work only if `CMAKE_BUILD_TYPE` set to `Debug`.

## AStylesheet

AStylesheet is a class used for applying styles on views.

### Basic usage

The example below defines a struct which constructor will be called on a program startup before the `main()` function is
called. It applies red background for all AButtons and blue border for ATextFields.

`Style.cpp`

```c++
// AStylesheet and declarations
#include <AUI/AUI/ASS.h>

#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>

// BackgroundSolid, any<> and other declarations are in ass namespace
using namespace ass;

struct Style {
    Style() {
        AStylesheet::inst().addRules({
            // first rule
            {
                t<AButton>(),
                BackgroundSolid { 0xff0000_rgb }
            },
            
            // second rule
            {
                t<ATextField>(),
                Border { 1_px, 0x0000ff_rgb }
            },
        });
    }
} inst;
```

The benefit of using way of defining struct is a simplicity: you don't have to define all styles in a function in the
embedded file and call the function from the `main()` function. Compiler automatically places call to the constructor of
this struct before the `main()` function call.