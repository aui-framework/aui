#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class AUIRecipe(ConanFile):
    name = "aui"
    version = "1.0"

    # Optional metadata
    license = "MPL2"
    author = "Alex2772 <alex2772sc@gmail.com>"
    url = "https://github.com/aui-framework/aui"
    description = "Declarative UI toolkit for modern C++20"
    topics = ("ui")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "aui.*", "cmake/*", "platform/*"
    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.16 <=4]")

    def requirements(self):
        self.requires("zlib/[>=1.2.11 <2]")
        self.requires("gtest/1.16.0")
        self.requires("benchmark/1.9.1")
        self.requires("fmt/9.1.0")
        self.requires("range-v3/0.12.0")
        self.requires("lunasvg/3.0.1")
        self.requires("opus/1.5.2")
        self.requires("soxr/0.1.3")

        if self.settings.os == "Linux":
            self.requires("libbacktrace/cci.20240730")


    def validate(self):
        if self.settings.get_safe("compiler.cppstd"):
            check_min_cppstd(self, 20)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={
            'AUIB_DISABLE': True
        })
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.bindirs = [] # cspell:disable-line
        self.cpp_info.set_property("cmake_file_name", "aui")
        self.cpp_info.set_property("cmake_find_mode", "config")
        self.cpp_info.set_property("cmake_build_modules", ["aui-config.cmake"])
