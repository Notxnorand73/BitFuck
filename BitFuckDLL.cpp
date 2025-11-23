#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

extern "C" {

// ------------------------- Types -------------------------
typedef void(*OutputCallback)(char c);
typedef int(*InputCallback)();

// ------------------------- BitLang VM -------------------------
class BitLangVM {
public:
    std::vector<int> memory;
    int pointer;
    std::unordered_map<std::string, std::string> macros;
    OutputCallback output_func;

    BitLangVM() : memory(1000, 0), pointer(0), output_func(nullptr) {}

    void define_macro(const char* name, const char* code) {
        macros[name] = code;
    }

    std::string expand_macros(const std::string& code) {
        std::string expanded = code;
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto& m : macros) {
                auto pos = expanded.find(m.first);
                if (pos != std::string::npos) {
                    expanded.replace(pos, m.first.length(), m.second);
                    changed = true;
                }
            }
        }
        return expanded;
    }

    void run(const std::string& code, InputCallback input_func) {
        std::string program = expand_macros(code);
        std::unordered_map<int, int> loop_map;
        std::vector<int> loop_stack;

        // Precompute loops
        for (size_t i = 0; i < program.size(); i++) {
            if (program[i] == '[') loop_stack.push_back(i);
            else if (program[i] == ']') {
                if (loop_stack.empty()) return;
                int start = loop_stack.back(); loop_stack.pop_back();
                loop_map[start] = i;
                loop_map[i] = start;
            }
        }
        if (!loop_stack.empty()) return; // unmatched [

        // Execute program
        for (size_t i = 0; i < program.size(); i++) {
            char cmd = program[i];
            switch (cmd) {
                case 'i': memory[pointer] = 1; break;
                case 'd': memory[pointer] = 0; break;
                case 'r': pointer++; if (pointer >= memory.size()) memory.push_back(0); break;
                case 'l': pointer--; if (pointer < 0) pointer = 0; break;
                case 'o': if (output_func) output_func(memory[pointer] + '0'); break;
                case 'e': if (input_func) memory[pointer] = input_func(); else memory[pointer] = 0; break;
                case '[': if (memory[pointer] == 0) i = loop_map[i]; break;
                case ']': if (memory[pointer] != 0) i = loop_map[i]; break;
            }
        }
    }

    int get_memory(int index) { return (index >= 0 && index < memory.size()) ? memory[index] : 0; }
    void reset_memory() { memory.assign(1000, 0); pointer = 0; }
};

// ------------------------- DLL Exports -------------------------
static BitLangVM vm;

// Set output callback and immediately print splash
__declspec(dllexport) void bitlang_set_output_callback(OutputCallback cb) {
    vm.output_func = cb;
    if (vm.output_func) {
        const char* splash = "Hello from BitLang!\n";
        while (*splash) vm.output_func(*splash++);
    }
}

__declspec(dllexport) void bitlang_define_macro(const char* name, const char* code) {
    vm.define_macro(name, code);
}

__declspec(dllexport) void bitlang_run_code(const char* code, InputCallback input_func) {
    vm.run(code, input_func);
}

__declspec(dllexport) int bitlang_get_memory(int index) {
    return vm.get_memory(index);
}

__declspec(dllexport) void bitlang_reset_memory() {
    vm.reset_memory();
}

} // extern "C"
