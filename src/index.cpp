#include <iostream>

#include "classes/semantics.hpp"
#include "classes/codegen.hpp"
#include "classes/parser.hpp"
#include "classes/lexer.hpp"

int main() {
    std::string code = R"(
    fn main() -> boolean {
        let x = 3;
        return x < 0 || x == 3;
    }
    )";
    Lexer lexer(code);
    auto tokens = lexer.lex();

    Parser parser(tokens);
    auto ast = parser.parse();

    SemanticAnalyzer analyzer;
    analyzer.analyze(ast);

    LLVMCodeGenerator generator;
    generator.generate(ast);

    generator.visualize_ir();
    return 0;
}