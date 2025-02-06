#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2024 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import json
import os
import subprocess
import sys


def set_github_env(name: str, value: str):
    env_file = os.getenv('GITHUB_ENV')

    with open(env_file, "a") as fos:
        fos.write(name)
        fos.write('=')
        fos.write(value)
        fos.write('\n')

def increment_rc_version(version: str) -> str:
    if "-rc." not in version:
        return f'{version}-rc.1'
    version, rc_index = version.split('-rc.')
    rc_index = int(rc_index) + 1
    return f'{version}-rc.{rc_index}'


assert increment_rc_version("v6.2.1") == "v6.2.1-rc.1"
assert increment_rc_version("v6.2.1-rc.24") == "v6.2.1-rc.25"

if __name__ == '__main__':
    latest = sys.argv[1]
    latest = increment_rc_version(latest)

    set_github_env('TAG_NAME', latest)
    set_github_env('RELEASE_NAME', latest)
