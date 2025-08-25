# GDB pretty formatter for the AUI project
#
# To enable, run the following command at the (lldb) prompt:
#      source (aui-dir)/gdb.py
#
# This can be automatically enabled at the start of every debugging session by creating a  ~/.gdbinit file which
# contains this command.

import re

_aui_disable_formatting = False

def _member_function_call(obj: gdb.Value, call: str):
    global _aui_disable_formatting
    _aui_disable_formatting = True
    out = gdb.parse_and_eval(f'(({obj.type}*){obj.address})->{call}')
    _aui_disable_formatting = False
    return out

class AOptionalPrinter(gdb.ValuePrinter):
    def __init__(self, val):
        self._val = val
        if val["mInitialized"]:
            self._contained_value = _member_function_call(val, "value()")
        else:
            self._contained_value = None

    def to_string(self):
        if self._contained_value is None:
            return f"[no contained value]"

    def display_hint(self):
        return 'string'

    def children(self):
        class Contained:
            def __init__(self, val):
                self._val = val

            def __iter__(self):
                return self

            def __next__(self):
                if self._val is None:
                    raise StopIteration
                retval = self._val
                self._val = None
                return ('[contained value]', retval)

        if self._contained_value is None:
            return Contained(None)
        return Contained(self._contained_value)

def AOptional_lookup(val):
    if _aui_disable_formatting:
        return
    pattern = re.compile(r'^AOptional<(.*)>$')
    if pattern.match(str(val.type.tag)):
        return AOptionalPrinter(val)

gdb.pretty_printers.append(AOptional_lookup)