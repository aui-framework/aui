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
