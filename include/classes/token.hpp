#pragma once

#include <string>
#include "enums/token_type.hpp"

class Token {
    public:
        std::string value;
        TokenType type;

        Token(TokenType type, std::string value) : type(type), value(std::move(value)) {}
};