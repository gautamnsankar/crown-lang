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

                if (initializer_type.kind != variable->type.kind && variable->type.kind != TypeKind::Unknown) {
                    throw std::runtime_error("Type mismatch in variable.");
                }

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

            if (const auto* variable = dynamic_cast<const AssignmentStatement*>(&statement)) {
                auto iterator = variables.find(variable->name);

                if (iterator == variables.end()) {
                    return;
                }

                auto value_type = analyze_expression(*variable->value);
                auto original_type = iterator->second.type;

                if (value_type.kind != original_type.kind) {
                    throw std::runtime_error(
                        "Original variable is of type " + original_type.to_string() +
                        ". Reassignment is of type " + value_type.to_string()
                    );
                }

                return;
            }

            if (const auto* loop = dynamic_cast<const WhileStatement*>(&statement)) {
                auto condition_type = analyze_expression(*loop->condition);

                if (condition_type.kind != TypeKind::Boolean) {
                    throw std::runtime_error("While loop condition must be a boolean");
                }

                return;
            }

            if (const auto* if_statement = dynamic_cast<const IfStatement*>(&statement)) {
                auto condition_type = analyze_expression(*if_statement->condition);

                if (condition_type.kind != TypeKind::Boolean) {
                    throw std::runtime_error("if statement condition must be a boolean");
                }

                return;
            }

            if (const auto* break_statement = dynamic_cast<const BreakStatement*>(&statement)) {
                return;
            }

            if (const auto* break_statement = dynamic_cast<const ContinueStatement*>(&statement)) {
                return;
            }

            throw std::runtime_error("Unknown statement.");
        }

        Type analyze_expression(const Expression& expression) {
            if (const auto* number = dynamic_cast<const NumberLiteral*>(&expression)) {
                return Type(number->number_type == NumberType::Double ? TypeKind::Double : TypeKind::Int);
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

            if (const auto* call = dynamic_cast<const FunctionCall*>(&expression)) {
                 auto iterator = functions.find(call->callee);

                if (iterator == functions.end()) {
                    throw std::runtime_error("Unknown function: " + call->callee);
                }

                const FunctionSymbol& fs = iterator->second;

                if (call->arguments.size() != fs.parameter_types.size()) {
                    throw std::runtime_error(
                        "Function " + call->callee + " expects " + std::to_string(fs.parameter_types.size()) + " arguments. but got "
                        + std::to_string(call->arguments.size())
                    );
                }

                for (std::size_t i = 0; i < call->arguments.size(); ++i) {
                    Type argument_type = analyze_expression(*call->arguments[i]);
                    Type expected_type = fs.parameter_types[i];

                    if (argument_type.kind != expected_type.kind) {
                        throw std::runtime_error(
                            "Argument " + std::to_string(i + 1) + 
                            " of function " + call->callee +
                            " has type" + argument_type.to_string() +
                            ", expected " + expected_type.to_string()
                        );
                    }
                }

                return fs.return_type;
            }

            if (const auto* binary = dynamic_cast<const BinaryExpression*>(&expression)) {
                Type left_type = analyze_expression(*binary->left);
                Type right_type = analyze_expression(*binary->right);

                bool is_numeric = (left_type.kind == TypeKind::Int || left_type.kind == TypeKind::Double) &&
                                (right_type.kind == TypeKind::Int || left_type.kind == TypeKind::Double);

                bool is_comparison = binary->op.value == "==" ||
                                     binary->op.value == "!=" ||
                                     binary->op.value == ">=" ||
                                     binary->op.value == "<=" ||
                                     binary->op.value == ">" ||
                                     binary->op.value == "<";

                bool is_logical = binary->op.value == "&&" || binary->op.value == "||";

                if (is_logical) {
                    if (left_type.kind != TypeKind::Boolean || right_type.kind != TypeKind::Boolean) {
                        throw std::runtime_error("Logical operators can only be performed on two booleans.");
                    }

                    return Type(TypeKind::Boolean);
                }

                if (is_comparison) {
                    if (!is_numeric) {
                        throw std::runtime_error(
                            "Cannot compare types " +
                            left_type.to_string() +
                            " and " +
                            right_type.to_string()
                        );
                    }

                    return Type(TypeKind::Boolean);
                }

                bool is_arithmetic = binary->op.value == "+" ||
                                     binary->op.value == "-" ||
                                     binary->op.value == "*" ||
                                     binary->op.value == "/";
                if (is_arithmetic) {
                    if (!is_numeric) {
                        throw std::runtime_error(
                            "Cannot perform arithmetic on types " +
                            left_type.to_string() +
                            " and " +
                            right_type.to_string()
                        );
                    }

                    if (left_type.kind == TypeKind::Double || right_type.kind == TypeKind::Double) {
                        return Type(TypeKind::Double);
                    }
                }

                return Type(TypeKind::Int);
            }

            if (const auto* unary = dynamic_cast<const UnaryExpression*>(&expression)) {
                auto right_type = analyze_expression(*unary->right);


                if (unary->op.value == "!") {
                    if (right_type.kind != TypeKind::Boolean) {
                        throw std::runtime_error("Cannot perform \"not\" on non boolean types.");
                    }

                    return Type(TypeKind::Boolean);
                }

                if (unary->op.value == "-") {
                    if (right_type.kind != TypeKind::Int && right_type.kind != TypeKind::Double) {
                        throw std::runtime_error("Cannot negate non numbers.");
                    }

                    return right_type;
                }

                throw std::runtime_error("Unknown unary operation.");
            }

            throw std::runtime_error("Unknown expression.");
        }

        void analyze(const Program& ast) {
            for (const auto& declaration : ast.declarations) {
                if (const auto* function = dynamic_cast<const FunctionDeclaration*>(declaration.get())) {
                    std::vector<Type> parameter_types;

                    for (const auto& parameter: function->parameters) {
                        parameter_types.push_back(parameter.type);
                    }

                    FunctionSymbol fs(parameter_types, function->name, function->return_type);
                    functions[function->name] = std::move(fs);
                }
            }
            for (const auto& declaration : ast.declarations) {
                analyze_declaration(*declaration);
            }
        }
};