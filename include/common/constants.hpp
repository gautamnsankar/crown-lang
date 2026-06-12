#pragma once

#include <string_view>
#include <array>

inline constexpr std::array<std::string_view, 5> OPERATORS = {"+", "-", "*", "/", "="};
inline constexpr std::array<std::string_view, 1> KEYWORDS = {"let"};

constexpr static inline unsigned char WHITESPACE = (unsigned char) 32;