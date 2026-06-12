#pragma once

#include <string>

enum class TokenType : unsigned char {
    RightCurlyBraces,
    LeftCurlyBraces,

    RightParenthesis,
    LeftParenthesis,
    
    BooleanLiteral,
    NumberLiteral,

    Identifier,
    EndOfFile,

    Operator,
    Keyword,
    String,
    
    Semicolon,
    Comma
};

class Token {
    public:
        std::string value;
        TokenType type;

        Token(TokenType type, std::string value) : type(type), value(std::move(value)) {}
};