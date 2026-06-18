#pragma once

#include <string_view>
#include <array>

inline constexpr std::array<std::string_view, 17> OPERATORS = {
    "+",
    "-",
    "*",
    "/",
    "=",
    "->",
    ":",
    "!",
    ".",
    "==",
    "!=",
    ">",
    "<",
    ">=",
    "<=",
    "&&",
    "||"
};
inline constexpr std::array<std::string_view, 11> KEYWORDS = {
    "let",
    "fn",
    "return",
    "while",
    "if",
    "else",
    "break",
    "continue",
    "extern",
    "import",
    "class"
};

constexpr static inline unsigned char WHITESPACE = (unsigned char) 32;