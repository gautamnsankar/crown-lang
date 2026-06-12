#pragma once

enum class TokenType : unsigned char {
    RightParenthesis,
    LeftParenthesis,
    BooleanLiteral,
    NumberLiteral,
    Identifier,
    Operator,
    Keyword,
    String
};