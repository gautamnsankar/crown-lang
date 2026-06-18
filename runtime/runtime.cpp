#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>

struct CrownString {
    std::size_t length;
    char* data;
};

// stdlib functions
extern "C" void print_int_raw(std::int64_t value) {
    std::cout << value << '\n';
}

extern "C" void print_boolean_raw(bool value) {
    std::cout << (value ? "true" : "false") << '\n';
}

extern "C" void print_double_raw(double value) {
    std::cout << value << '\n';
}

// string functions
extern "C" CrownString* new_string(const char* text) {
    auto *s = new CrownString;

    s->length = std::strlen(text);
    s->data = new char[s->length + 1];

    std::memcpy(s->data, text, s->length + 1);

    return s;
}

extern "C" void delete_string(CrownString* string) {
    if (!string) {
        return;
    }

    delete[] string->data;
    delete string;
}

extern "C" std::size_t string_length(CrownString* string) {
    return string->length;
}

extern "C" void print_string_raw(CrownString* string) {
    if (!string) {
        return;
    }

    std::cout << string->data << '\n';
}