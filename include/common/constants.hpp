#pragma once

#include <string_view>
#include <array>

inline constexpr std::array<std::string_view, 13> OPERATORS = {
    "+",
    "-",
    "*",
    "/",
    "=",
    "->",
    ":",
    "==",
    "!=",
    ">",
    "<",
    ">=",
    "<="
};
inline constexpr std::array<std::string_view, 5> KEYWORDS = {
    "let",
    "fn",
    "return",
    "while",
    "for"
};

constexpr static inline unsigned char WHITESPACE = (unsigned char) 32;