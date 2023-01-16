#  AUI Framework - Declarative UI toolkit for modern C++17
#  Copyright (C) 2020-2023 Alex2772
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library. If not, see <http://www.gnu.org/licenses/>.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library. If not, see <http://www.gnu.org/licenses/>.

# LLDB pretty formatter for the AUI project
#
# To enable, run the following command at the (lldb) prompt:
#      command script import (aui-dir)/lddb.py
#
# This can be automatically enabled at the start of every debugging session by creating a  ~/.lldbinit file which
# contains this command.

import lldb

def __lldb_init_module(debugger, internal_dict):
    # aui::lazy<T>
    debugger.HandleCommand(f'type summary add -x ^aui::lazy<(.+)>$ -F {__name__}.aui_lazy')

    # aui::range<T>
    debugger.HandleCommand(f'type summary add -x ^aui::range<(.+)>$ -F {__name__}.aui_range')

    # --inline-children shows the values of children in summaries, -O only shows values, not keys
    debugger.HandleCommand(f'type summary add -x ^AOptional<.+>$ --inline-children -O')
    debugger.HandleCommand(f'type synthetic add -x ^AOptional<.+>$ -l {__name__}.Optional')


class Optional(object):
    def __init__(self, value, internal_dict):
        self.initialized = False
        self.value = value
        self.valueType = None

    def num_children(self):
        if self.initialized:
            return 1
        return 0

    def has_children(self):
        return self.initialized

    def get_child_at_index(self, index):
        if index == 0:
            return self.value.CreateChildAtOffset("value", 0, self.valueType)
        return None

    def update(self):
        self.initialized = self.value.GetChildMemberWithName("mInitialized").GetValueAsUnsigned() != 0
        if self.initialized:
            self.valueType = self.value.EvaluateExpression("value()").GetType()


def aui_lazy(value, internalDict):
    return value.GetChildMemberWithName("value").GetSummary()


def aui_range(value, internalDict):
    return f'size = {value.EvaluateExpression("size()").GetSummary()}'
