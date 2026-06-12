#include <iostream>

#include "classes/parser.hpp"
#include "classes/lexer.hpp"

void visualize_tokens(const std::vector<Token>& tokens) {
    for (const Token& t : tokens) {
        std::cout << "Type: " << (int) t.type << '\n';
        std::cout << "Value: " << t.value << '\n';

        std::cout << '\n';
    }
}

void print_indent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "   ";
    }
}

void visualize_expression(const Expression* expr, int indent);

void visualize_statement(const Statement* stmt, int indent);

void visualize_declaration(const Declaration* decl, int indent);

void visualize_ast(const Program& program) {
    std::cout << "Program\n";

    for (const auto& declaration : program.declarations) {
        visualize_declaration(declaration.get(), 1);
    }
}

void visualize_declaration(const Declaration* decl, int indent) {
    if (const auto* fn = dynamic_cast<const FunctionDeclaration*>(decl)) {
        print_indent(indent);
        std::cout << "FunctionDeclaration name=" << fn->name
                  << " return_type=" << fn->return_type.to_string() << "\n";

        print_indent(indent + 1);
        std::cout << "Parameters\n";

        for (const auto& param : fn->args) {
            print_indent(indent + 2);
            std::cout << "Parameter name=" << param.name
                      << " type=" << param.type.to_string()
                      << "\n";
        }

        if (fn->body) {
            visualize_statement(fn->body.get(), indent + 1);
        }

        return;
    }

    print_indent(indent);
    std::cout << "UnknownDeclaration\n";
}

void visualize_statement(const Statement* stmt, int indent) {
    if (const auto* block = dynamic_cast<const BlockStatement*>(stmt)) {
        print_indent(indent);
        std::cout << "BlockStatement\n";

        for (const auto& statement : block->statements) {
            visualize_statement(statement.get(), indent + 1);
        }

        return;
    }

    if (const auto* var = dynamic_cast<const VariableDeclaration*>(stmt)) {
        print_indent(indent);
        std::cout << "VariableDeclaration name=" << var->name
                  << " type=" << var->type.to_string()
                  << "\n";

        if (var->value) {
            visualize_expression(var->value.get(), indent + 1);
        }

        return;
    }

    if (const auto* ret = dynamic_cast<const ReturnStatement*>(stmt)) {
        print_indent(indent);
        std::cout << "ReturnStatement\n";

        if (ret->value) {
            visualize_expression(ret->value.get(), indent + 1);
        }

        return;
    }

    print_indent(indent);
    std::cout << "UnknownStatement\n";
}

void visualize_expression(const Expression* expr, int indent) {
    if (const auto* number = dynamic_cast<const NumberLiteral*>(expr)) {
        print_indent(indent);
        std::cout << "NumberLiteral value=" << number->value << "\n";
        return;
    }

    if (const auto* boolean = dynamic_cast<const BooleanLiteral*>(expr)) {
        print_indent(indent);
        std::cout << "BooleanLiteral value="
                  << (boolean->value ? "true" : "false")
                  << "\n";
        return;
    }

    if (const auto* binary = dynamic_cast<const BinaryExpression*>(expr)) {
        print_indent(indent);
        std::cout << "BinaryExpression op=" << binary->op.value << "\n";

        print_indent(indent + 1);
        std::cout << "Left\n";
        visualize_expression(binary->left.get(), indent + 2);

        print_indent(indent + 1);
        std::cout << "Right\n";
        visualize_expression(binary->right.get(), indent + 2);

        return;
    }

    if (const auto* variable = dynamic_cast<const VariableReference*>(expr)) {
        print_indent(indent);
        std::cout << "VariableReference name=" << variable->name << "\n";
        return;
    }

    print_indent(indent);
    std::cout << "UnknownExpression\n";
}

int main() {
    std::string code = "fn main() -> void {"
                        "   let x = 1;"
                        "   return (x + 1) * 2;"
                       "}";
    Lexer lexer(code);

    std::vector<Token> tokens = lexer.lex();
    Parser parser(tokens);

    // visualize_tokens(tokens);

    Program ast = parser.parse();
    std::cout << "Compilation finished." << '\n';

    visualize_ast(ast);

    return 0;
}