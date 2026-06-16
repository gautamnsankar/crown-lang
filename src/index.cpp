#include <iostream>

#include "classes/semantics.hpp"
#include "classes/codegen.hpp"
#include "classes/parser.hpp"
#include "classes/lexer.hpp"

int main() {
    std::string code = R"(
    fn test(a: int, b: int) -> void {
        return;
    }
    
    fn main() -> int {
        while (true) {
            test(1, 2);
        }
        
        return 0;
    }
    )";
    Lexer lexer(code);
    auto tokens = lexer.lex();

    Parser parser(tokens);
    auto ast = parser.parse();
    
    // Lexer::visualize_tokens(tokens);
    Parser::visualize_ast(ast);

    // SemanticAnalyzer analyzer;
    // analyzer.analyze(ast);

    LLVMCodeGenerator generator;
    generator.generate(ast);

    generator.visualize_ir();
    return 0;
}