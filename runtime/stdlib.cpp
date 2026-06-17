#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>

extern "C" void print_int_raw(std::int64_t value) {
    std::cout << value << '\n';
}

extern "C" void print_boolean_raw(bool value) {
    std::cout << (value ? "true" : "false") << '\n';
}

extern "C" void print_double_raw(double value) {
    std::cout << value << '\n';
}

extern "C" void print_string_raw(const char* value) {
    std::cout << value << '\n';
}