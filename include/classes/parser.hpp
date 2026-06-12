#pragma once

#include <vector>

#include "classes/token.hpp"
#include "common/ast.hpp"

class Parser {
    public:
        std::vector<Token> tokens;
        std::size_t cursor;
};