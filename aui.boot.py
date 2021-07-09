# ======================================================================================================================
# Copyright (c) 2021 Alex2772
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Original code located at https://github.com/Alex2772/aui
# ======================================================================================================================

import argparse
import multiprocessing
import os
import re
import shutil
import subprocess
import sys
from enum import Enum


def print_splash():
    print('                                                          ###')
    print('                                                          ###')
    print('                                                             ')
    print('                      ####   ###                    ###   ###')
    print('                    ######   ###                    ###   ###')
    print('                  ###  ###   ###                    ###   ###')
    print('                ###    ###   ###                    ###   ###')
    print('              ###      ###   ###                    ###   ###')
    print('            ###        ###   ###                    ###   ###')
    print('          ###          ###   ###                    ###   ###')
    print('        ##################   ###                    ###   ###')
    print('      ###              ###    ###                  ###    ###')
    print('    ###                ###     ###                ###     ###')
    print('  ###                  ###       #####        #####       ###')
    print('###                    ###           ##########           ###')
    print('')


class OS(Enum):
    linux = 'Linux'
    windows = 'Windows'
    unknown = 'Unknown'

    def __str__(self):
        return self.value


target_os = OS.unknown

class ExecuteError(RuntimeError):
    command = []
    output = ""

    def __init__(self, command, output):
        self.command = command
        self.output = output

    def __str__(self) -> str:
        return 'failed to execute command {}: {}'.format(self.command, self.output)


def execute_command(cmd):
    try:
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True)
        process.wait(10)
        exit_code = process.poll()
        if exit_code == 0:
            return process.stdout.readline()
        raise ExecuteError(cmd, "exit code: {}, {}".format(exit_code, process.stdout.readline()))
    except OSError as e:
        raise ExecuteError(cmd, e.strerror)


def check_program_version(cmd):
    try:
        return execute_command(cmd)
    except ExecuteError as e:
        print(e.output)
    except RuntimeError as e:
        print(e)

    return False


class Action(Enum):
    boot = 'boot'
    new = 'new'

    def __str__(self):
        return self.value


def main():
    parser = argparse.ArgumentParser(description='AUI project creator')

    parser.add_argument('action',
                        type=Action,
                        choices=list(Action),
                        help='boot - compile and install AUI\n'
                             'new - generate a new AUI project')

    parser.add_argument('--name',
                        type=str,
                        required=False,
                        dest='name',
                        help='new project\'s name')

    parser.add_argument('--cores',
                        type=int,
                        dest='core_count',
                        default=multiprocessing.cpu_count(),
                        help='number of cores used in compilation')

    args = parser.parse_args()

    # determine target OS
    global target_os
    if sys.platform == 'linux' or sys.platform == 'linux2':
        target_os = OS.linux
    elif sys.platform == 'win32':
        target_os = OS.windows

    print("System information")
    print("OS:", os.name)
    print("Path:", os.getcwd())
    print("Platform:", sys.platform, '( treated as', target_os, ')')

    if target_os == OS.unknown:
        print("Target OS is unsupported, shutting down")
        return -1

    if args.action == Action.boot:
        print("Checking required programs...")

        required_programs = [
            # name, command to check version, version word index, helper string for linux, helper string for windows
            (
                'CMake',
                ['cmake', '--version'],
                2,
                'Download it from https://cmake.org/download/ or sudo apt install cmake',
                'Download it from https://cmake.org/download/'
            ),
            (
                'Git',
                ['git', '--version'],
                2,
                'sudo apt install git',
                'Download it from https://git-scm.com/'
            ),
        ]

        if target_os == OS.linux:
            required_programs.append((
                'G++ compiler',
                ['g++', '--version'],
                2,
                'sudo apt install g++ build-essential'
            ))

        unsatisfied_programs = []

        for p in required_programs:
            print(p[0], end='')
            print('...', end='')
            result = check_program_version(p[1])
            if result is False:
                unsatisfied_programs.append(p)
            else:
                if result.endswith('\n'):
                    result = result[:-1]
                splt = result.split(' ')
                if len(splt) <= p[2]:
                    print(result)
                else:
                    print(splt[p[2]])

        if len(unsatisfied_programs) > 0:
            print('Some required programs are not installed!')
            for p in unsatisfied_programs:
                print(p[0], ':', sep='')
                if target_os == OS.linux:
                    print(p[3])
                elif target_os == OS.windows:
                    print(p[4])

            return -1

        # check if getcwd() points to aui working dir

        aui_dir = os.getcwd()
        if not os.path.isdir(os.path.join(aui_dir, 'AUI.Core')):
            print("Please run this script from the AUI source directory.")
            return

        print_splash()

        build_dir = os.path.join(aui_dir, 'aui_boot_build')

        # remove previous build dir
        if os.path.isdir(build_dir):
            shutil.rmtree(build_dir)

        os.makedirs(build_dir)
        process = subprocess.Popen(['cmake', '..'], cwd=build_dir)
        process.wait(1000)

        if process.poll() != 0:
            return -1

        process = subprocess.Popen(['make', '-j{}'.format(args.core_count)], cwd=build_dir)
        process.wait(1000)
        if process.poll() != 0:
            return -1

        print('Please type your password below in order to install.')
        process = subprocess.Popen(['sudo', 'make', 'install'], cwd=build_dir)
        process.wait(1000)

        if process.poll() != 0:
            return -1
    elif args.action == Action.new:
        if args.name is None:
            print('When creating a new project, --name= is required.')
            return -1

        if not re.match(r'^[\w_]+$', str(args.name)):
            print("Project name may only contain latin letters, numbers and underscores.")
            return -1

        print_splash()
        print('Generating project...')

        project_dir = os.path.join(os.getcwd(), args.name)
        if os.path.isdir(project_dir):
            shutil.rmtree(project_dir)

        os.makedirs(project_dir)

        # generate CMakeLists.txt
        with open(os.path.join(project_dir, 'CMakeLists.txt'), 'w') as f:
            f.write('''cmake_minimum_required(VERSION 3.16)
project({name})

# Link AUI
find_package(AUI REQUIRED)

AUI_Executable({name})

# Link required libs
target_link_libraries({name} PRIVATE AUI.Core)
# target_link_libraries({name} PRIVATE AUI.Views)'''.format(name=args.name))

        # generate main.cpp
        src_dir = os.path.join(project_dir, 'src')
        os.makedirs(src_dir)

        with open(os.path.join(src_dir, 'main.cpp'), 'w') as f:
            f.write('''#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/Entry.h>

AUI_ENTRY {{
    ALogger::info("Hello from {{}}!"_as.format("{}"));
    return 0;
}}
            '''.format(args.name))



    return 0

if __name__ == '__main__':
    sys.exit(main())
