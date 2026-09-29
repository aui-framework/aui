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

for file in *; do if [[ ! $file =~ ^(bin|lib|share)$ ]]; then echo "Unexpected file: $file"; exit -1; fi; done
if [[ -d share ]]; then
    # freedesktop entries: .desktop files and hicolor icons (png/svg)
    while IFS= read -r file; do
        if [[ ! $file =~ ^share/applications/[^/]+\.desktop$ \
           && ! $file =~ ^share/icons/hicolor/([0-9]+x[0-9]+|scalable)/apps/[^/]+\.(png|svg)$ ]]; then
            echo "Unexpected file: $file"; exit -1
        fi
    done < <(find share -type f)
fi
for file in bin/*; do if [[ ! $file == bin/test_project ]]; then echo "Unexpected file: $file"; exit -1; fi; done
for file in lib/*; do
    if [[ ! $file =~ (\.so[\.0-9]*(zlib-ng)?|lib\/\*)$ ]]; then
        echo "Unexpected file: $file"
        exit -1
    fi
done
