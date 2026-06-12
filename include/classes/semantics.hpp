#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "classes/parser.hpp"

class FunctionSymbol {
    public:
        std::vector<Type> parameter_types;
        std::string name;
        Type return_type;
};

class VariableSymbol {
    public:
        std::string name;
        Type type;
};

class SemanticAnalyzer {
    private:
        std::unordered_map<std::string, FunctionSymbol> functions;
        std::unordered_map<std::string, VariableSymbol> variables;

        Type current_return_type;

    public:
        SemanticAnalyzer() = default;

        void analyze_declaration(const Declaration& declaration) {
            if (const auto* function = dynamic_cast<const FunctionDeclaration*>(&declaration)) {
                analyze_function(*function);
                return;
            }

            throw std::runtime_error("Unknown declaration.");
        }

        void analyze_function(const FunctionDeclaration& function) {
            variables.clear();

            for (const auto& parameter : function.parameters) {
                variables[parameter.name] = VariableSymbol(parameter.name, parameter.type);
            }

            current_return_type = function.return_type;
            analyze_block(*function.body);
        }

        void analyze_block(const BlockStatement& block) {
            for (const auto& statement : block.statements) {
                analyze_statement(*statement);
            }
        }

        void analyze_statement(const Statement& statement) {
            if (const auto* variable = dynamic_cast<const VariableDeclaration*>(&statement)) {
                Type initializer_type = analyze_expression(*variable->value);
                Type final_type = variable->type.kind == TypeKind::Unknown ? initializer_type : variable->type;

                variables[variable->name] = VariableSymbol(variable->name, final_type);
                return;
            }
            
            if (const auto* ret = dynamic_cast<const ReturnStatement*>(&statement)) {
                if (!ret->value) {
                    if (current_return_type.kind != TypeKind::Void) {
                        throw std::runtime_error("Non void function must return a value.");
                    }

                    return;
                }

                Type value_type = analyze_expression(*ret->value);

                if (current_return_type.kind == TypeKind::Void) {
                    throw std::runtime_error("Void function cannot return a value");
                }

                if (value_type.kind != current_return_type.kind) {
                    throw std::runtime_error("Return type mismatch.");
                }

                return;
            }

            throw std::runtime_error("Unknown statement.");
        }

        Type analyze_expression(const Expression& expression) {
            if (const auto* number = dynamic_cast<const NumberLiteral*>(&expression)) {
                return Type(number->number_type == NumberType::Float ? TypeKind::Float : TypeKind::Int);
            }

            if (dynamic_cast<const BooleanLiteral*>(&expression)) {
                return Type(TypeKind::Boolean);
            }

            if (const auto* string = dynamic_cast<const StringLiteral*>(&expression)) {
                return Type(TypeKind::String);
            }

            if (const auto* variable = dynamic_cast<const VariableReference*>(&expression)) {
                auto iterator = variables.find(variable->name);

                if (iterator == variables.end()) {
                    throw std::runtime_error("Unknown variable: " + variable->name);
                }

                return iterator->second.type;
            }

            if (const auto* binary = dynamic_cast<const BinaryExpression*>(&expression)) {
                Type left_type = analyze_expression(*binary->left);
                Type right_type = analyze_expression(*binary->right);

                bool is_valid = (left_type.kind == TypeKind::Int || left_type.kind == TypeKind::Float) &&
                                (right_type.kind == TypeKind::Int || left_type.kind == TypeKind::Float);

                if (!is_valid) {
                    throw std::runtime_error("Cannot perform arithmetic on types " + left_type.to_string() + " and " + right_type.to_string());
                }

                return Type(left_type.kind);
            }

            throw std::runtime_error("Unknown expression.");
        }

        void analyze(const Program& ast) {
            for (const auto& declaration : ast.declarations) {
                analyze_declaration(*declaration);
            }
        }
};