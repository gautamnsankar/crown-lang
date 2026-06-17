#include <iostream>

#include "classes/semantics.hpp"
#include "classes/codegen.hpp"
#include "classes/imports.hpp"
#include "classes/parser.hpp"
#include "classes/lexer.hpp"

std::string read_file(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error("Cannot find file.");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

int main(int argc, char** argv) {
    if (argc != 4 || std::string(argv[2]) != "-o") {
        std::cerr << "Usage: ./compiler <input> -o <output>\n";
        return 1;
    }

    std::string input = argv[1];
    std::string output = argv[3];

    std::string code = read_file(input);

    Lexer lexer(code);
    auto tokens = lexer.lex();

    Parser parser(tokens);
    auto ast = parser.parse();

    ImportResolver resolver;
    ast = resolver.resolve(std::move(ast));

    SemanticAnalyzer analyzer;
    analyzer.analyze(ast);

    LLVMCodeGenerator codegen;
    codegen.generate(ast);

    codegen.create_executable(output);
}