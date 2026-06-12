#include <iostream>
#include "classes/lexer.hpp"

void visualize_tokens(const std::vector<Token>& tokens) {
    for (const Token& t : tokens) {
        std::cout << "Type: " << (int) t.type << '\n';
        std::cout << "Value: " << t.value << '\n';

        std::cout << '\n';
    }
}

int main() {
    std::string code = "let x = `Hello WORLD.`";
    std::cout << code << '\n';
    Lexer l(code);

    std::vector<Token> a = l.lex();
    visualize_tokens(a);
    return 0;
}