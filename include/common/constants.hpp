#pragma once

#include <string_view>
#include <array>

inline constexpr std::array<std::string_view, 7> OPERATORS = {"+", "-", "*", "/", "=", "->", ":"};
inline constexpr std::array<std::string_view, 3> KEYWORDS = {"let", "fn", "return"};

constexpr static inline unsigned char WHITESPACE = (unsigned char) 32;