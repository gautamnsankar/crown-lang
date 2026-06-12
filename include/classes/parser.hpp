#pragma once

#include <vector>

#include "classes/token.hpp"
#include "classes/type.hpp"
#include "common/ast.hpp"

class Parser {
    private:
        std::vector<Token> tokens;
        std::size_t cursor;

        bool is_end() {
            return (cursor >= tokens.size() || tokens[cursor].type == TokenType::EndOfFile);
        }

        const Token& peek() {
            if ((cursor + 1) >= tokens.size()) {
                return tokens[tokens.size() - 1];
            }

            return tokens[cursor + 1];
        }

        const Token& current() {
            if (is_end()) {
                return tokens[tokens.size() - 1];
            }

            return tokens[cursor];
        }

        const Token& advance() {
             if (is_end()) {
                return tokens[tokens.size() - 1];
             }

            const Token& t = current();
            ++cursor;

            return t;
        }

        const Token& expect(TokenType type) {
            const Token& c = current();

            if (c.type != type) {
                throw std::runtime_error(
                    "Expected: " + std::to_string(static_cast<int>(type)) +
                    " Got: " + std::to_string(static_cast<int>(c.type))
                );
            }

            return advance();
        }

        const Token& expect(TokenType type, std::string value) {
           const Token& c = current();

            if (c.type != type) {
                throw std::runtime_error(
                    "Expected: " + std::to_string(static_cast<int>(type)) +
                    " Got: " + std::to_string(static_cast<int>(c.type))
                );
            }

             if (c.value != value) {
                throw std::runtime_error(
                    "Expected: " + value +
                    " Got: " + c.value
                );
            }

            return advance();
        }

    public:
        Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), cursor(0) {}

        Type parse_type() {
            const Token& token = expect(TokenType::Identifier);

            if (token.value == "void") {
                return Type(TypeKind::Void);
            }

            if (token.value == "int") {
                return Type(TypeKind::Int);
            }

            if (token.value == "boolean") {
                return Type(TypeKind::Boolean);
            }

            return Type(TypeKind::Unknown);
        }

        bool is_binary_operator(const Token& token) {
            if (token.type != TokenType::Operator) {
                return false;
            }

            return token.value == "+" ||
                   token.value == "-" ||
                   token.value == "*" ||
                   token.value == "/";
        }

        int precedence(const Token& token) {
            if (token.type != TokenType::Operator) {
                return -1;
            }

            if (token.value == "==" || token.value == "!=" || 
                token.value == "<" || token.value == "<=" ||
                token.value == ">" || token.value == ">=") {
                return 1;
            }

            if (token.value == "+" || token.value == "-") {
                return 2;
            }

            if (token.value == "*" || token.value == "/") {
                return 3;
            }

            return -1;
        }

        std::unique_ptr<Expression> parse_primary_expression() {
            const Token& token = current();

            if (token.type == TokenType::NumberLiteral) {
                const Token& token = advance();

                NumberType type = token.value.find('.') != std::string::npos ? NumberType::Float : NumberType::Integer;
                return std::make_unique<NumberLiteral>(token.value, type);
            }

            if (token.value == "true" || token.value == "false") {
                advance();
                return std::make_unique<BooleanLiteral>(token.value == "true" ? true : false);
            }

            if (token.type == TokenType::LeftParenthesis) {
                advance();
                auto expression = parse_expression();
                expect(TokenType::RightParenthesis);
                return expression;
            }

            if (token.type == TokenType::Identifier) {
                advance();
                return std::make_unique<VariableReference>(token.value);
            }

            throw std::runtime_error("Expected an expression.");
        }

        std::unique_ptr<Expression> parse_expression(int min_precedence = 0) {
            auto left = parse_primary_expression();

            while (true) {
                const Token& token = current();
                int op_precedence = precedence(token);

                if (op_precedence < min_precedence) {
                    break;
                }

                Token op_token = advance();
                auto right = parse_expression(op_precedence + 1);

                left = std::make_unique<BinaryExpression>(
                    std::move(left),
                    token,
                    std::move(right)
                );
            }

            return left;
        }

        std::unique_ptr<VariableDeclaration> parse_variable_declaration() {
            expect(TokenType::Keyword, "let");

            Type type(TypeKind::Unknown);

            const Token& variable_name = expect(TokenType::Identifier);

            if (current().value == ":") {
                expect(TokenType::Operator, ":");
                type = parse_type();
            }

            expect(TokenType::Operator, "=");

            return std::make_unique<VariableDeclaration>(
                variable_name.value,
                parse_expression(),
                type
            );
        }

        std::unique_ptr<ReturnStatement> parse_return_statement() {
            expect(TokenType::Keyword, "return");

            auto value = parse_expression();
            return std::make_unique<ReturnStatement>(std::move(value));
        }

        std::unique_ptr<Statement> parse_statement() {
            if (current().value == "let") {
                return parse_variable_declaration();
            }

            if (current().value == "return") {
                return parse_return_statement();
            }

            throw std::runtime_error("Expected a statement.");
        }

        std::unique_ptr<BlockStatement> parse_block() {
            expect(TokenType::LeftCurlyBraces);

            auto block = std::make_unique<BlockStatement>();

            while (current().type != TokenType::RightCurlyBraces && current().type != TokenType::EndOfFile) {
                block->statements.push_back(parse_statement());
            }

            expect(TokenType::RightCurlyBraces);

            return block;
        }

        std::unique_ptr<Declaration> parse_top_level_function() {
            if (current().value == "fn") {
                return parse_function_declaration();
            }

            throw std::runtime_error("Could not find entry point.");
        }

        std::unique_ptr<FunctionDeclaration> parse_function_declaration() {
            const Token& fn_keyword = expect(TokenType::Keyword, "fn");
            const Token& fn_name = expect(TokenType::Identifier);

            expect(TokenType::LeftParenthesis);
            std::vector<FunctionParameter> parameters;

            while (current().type != TokenType::RightParenthesis) {
                const Token& name = expect(TokenType::Identifier);

                expect(TokenType::Operator, ":");

                Type type = parse_type();
                parameters.push_back(FunctionParameter(name.value, type));

                if (current().type != TokenType::RightParenthesis) {
                    expect(TokenType::Comma);
                }
            }

            expect(TokenType::RightParenthesis);
            expect(TokenType::Operator, "->");

            Type return_type = parse_type();
            auto body = parse_block();

            return std::make_unique<FunctionDeclaration>(
                fn_name.value,
                std::move(parameters),
                return_type,
                std::move(body)
            );
        }

        Program parse() {
            Program ast;

            while (!is_end()) {
                ast.declarations.push_back(parse_function_declaration());
            }

            return ast;
        }
};