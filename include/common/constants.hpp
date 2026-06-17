#pragma once

#include <string_view>
#include <array>

inline constexpr std::array<std::string_view, 16> OPERATORS = {
    "+",
    "-",
    "*",
    "/",
    "=",
    "->",
    ":",
    "!",
    "==",
    "!=",
    ">",
    "<",
    ">=",
    "<=",
    "&&",
    "||"
};
inline constexpr std::array<std::string_view, 10> KEYWORDS = {
    "let",
    "fn",
    "return",
    "while",
    "if",
    "else",
    "break",
    "continue",
    "extern",
    "import"
};

constexpr static inline unsigned char WHITESPACE = (unsigned char) 32;