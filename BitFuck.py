import ctypes
import os

# -------------------- Load DLL --------------------
dll_path = os.path.join(os.path.dirname(__file__), "BitFuckDLL.dll")
_bitfuck = ctypes.CDLL(dll_path)

# Callback types
OUTPUT_FUNC = ctypes.CFUNCTYPE(None, ctypes.c_char)
INPUT_FUNC = ctypes.CFUNCTYPE(ctypes.c_int)

# -------------------- BitLang Wrapper --------------------
class BitFuck:
    def __init__(self, memory_size=1000, commands=None):
        # Callbacks
        self._output_cb = None
        self._input_cb = None
        
        # Commands dictionary: friendly names → actual BitLang commands
        self.commands = commands or {
            "ON": "i",
            "OFF": "d",
            "RIGHT": "r",
            "LEFT": "l",
            "OUTPUT": "o",
            "INPUT": "e",
            "LOOP_START": "[",
            "LOOP_END": "]"
        }
        
        self.memory_size = memory_size
        _bitfuck.bitfuck_set_memory_size(memory_size)

    # -------------------- Callbacks --------------------
    def set_output(self, func):
        """Set output callback; splash text prints immediately"""
        self._output_cb = OUTPUT_FUNC(func)
        _bitfuck.bitfuck_set_output_callback(self._output_cb)

    def set_input(self, func):
        """Set input callback"""
        self._input_cb = INPUT_FUNC(func)

    # -------------------- Macros --------------------
    def define_macro(self, name, code):
        """Define a macro"""
        _bitfuck.bitfuck_define_macro(name.encode('utf-8'), code.encode('utf-8'))

    # -------------------- Code Translation --------------------
    def translate_code(self, code):
        """Replace friendly names with actual Bitfuck commands"""
        for key, char in self.commands.items():
            code = code.replace(key, char)
        return code

    # -------------------- Run Code --------------------
    def run(self, code):
        code = self.translate_code(code)
        if not self._input_cb:
            self._input_cb = INPUT_FUNC(lambda: 0)  # default input = 0
        _bitfuck.bitfuck_run_code(code.encode('utf-8'), self._input_cb)

    # -------------------- Memory Access --------------------
    def get_memory(self, index):
        return _bitfuck.bitfuck_get_memory(index)

    def reset_memory(self):
        _bitfuck.bitfuck_reset_memory()

# -------------------- Helper Function to Load Macros from File --------------------
def load_macros_from_file(bf, path):
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if "=" in line:
                name, code = line.split("=", 1)
                bf.define_macro(name, code)
