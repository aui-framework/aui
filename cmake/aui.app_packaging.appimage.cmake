# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# this CMake file is intended to be included by aui_app.

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "AUI_APPIMAGE packaging is supported on Linux only")
endif ()

# AppImage is produced by a custom script (see aui_app, DESKTOP LINUX section) which is run by CPack's External generator.
aui_set_cpack_generator(External)
set(CPACK_EXTERNAL_ENABLE_STAGING NO)
set(AUI_APPIMAGE_ENABLED TRUE)
