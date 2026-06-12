#include <iostream>

#include "classes/semantics.hpp"
#include "classes/parser.hpp"
#include "classes/lexer.hpp"


int main() {
    std::string code = "fn main() -> string {"
                       "    let a = \"Hello\";"
                       "    return a;"
                       "}";

    Lexer lexer(code);
    auto tokens = lexer.lex();

    Parser parser(tokens);
    auto ast = parser.parse();

    // Lexer::visualize_tokens(tokens);
    Parser::visualize_ast(ast);

    SemanticAnalyzer analyzer;
    analyzer.analyze(ast);

    return 0;
}