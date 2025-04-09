#
# AUI Framework - Declarative UI toolkit for modern C++20
# Copyright (C) 2020-2025 Alex2772 and Contributors
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

for file in *; do if [[ ! $file =~ ^(bin|lib)$ ]]; then echo "Unexpected file: $file"; exit -1; fi; done
for file in bin/*; do if [[ ! $file == bin/test_project ]]; then echo "Unexpected file: $file"; exit -1; fi; done
for file in lib/*; do if [[ ! $file =~ (\.so[\.0-9]*|lib/\*)$ ]]; then echo "Unexpected file: $file"; exit -1; fi; done
