# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# this CMake file is intended to be included by aui_app.

aui_set_cpack_generator(TGZ)

set(_aui_package_file_name ${_aui_package_file_name}-portable) # append -portable suffix
