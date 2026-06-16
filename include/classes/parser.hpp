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

            if (token.value == "string") {
                return Type(TypeKind::String);
            }

            if (token.value == "int") {
                return Type(TypeKind::Int);
            }

            if (token.value == "double") {
                return Type(TypeKind::Double);
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

            if (token.value == "||") {
                return 1;
            }

            if (token.value == "&&") {
                return 2;
            }

            if (token.value == "==" || token.value == "!=" || 
                token.value == "<" || token.value == "<=" ||
                token.value == ">" || token.value == ">=") {
                return 3;
            }

            if (token.value == "+" || token.value == "-") {
                return 4;
            }

            if (token.value == "*" || token.value == "/") {
                return 5;
            }

            return -1;
        }

        std::unique_ptr<Expression> parse_unary_expression() {
            if (current().type == TokenType::Operator && current().value == "!" || current().value == "-") {
                Token op = advance();
                auto right = parse_unary_expression();

                return std::make_unique<UnaryExpression>(op, std::move(right));
            }

            return parse_primary_expression();
        }

        std::unique_ptr<Expression> parse_primary_expression() {
            const Token& token = current();

            if (token.type == TokenType::NumberLiteral) {
                const Token& token = advance();

                NumberType type = token.value.find('.') != std::string::npos ? NumberType::Double : NumberType::Integer;
                return std::make_unique<NumberLiteral>(token.value, type);
            }

            if (token.value == "true" || token.value == "false") {
                advance();
                return std::make_unique<BooleanLiteral>(token.value == "true" ? true : false);
            }

            if (token.type == TokenType::String) {
                const Token& token = advance();
                return std::make_unique<StringLiteral>(token.value);
            }

            if (token.type == TokenType::LeftParenthesis) {
                advance();
                auto expression = parse_expression();
                expect(TokenType::RightParenthesis);
                return expression;
            }

            if (token.type == TokenType::Identifier && peek().type != TokenType::LeftParenthesis) {
                advance();
                return std::make_unique<VariableReference>(token.value);
            }

            if (token.type == TokenType::Identifier && peek().type == TokenType::LeftParenthesis) {
                std::vector<std::unique_ptr<Expression>> arguments;
                std::string callee = token.value;

                advance();
                expect(TokenType::LeftParenthesis);

                while (current().type != TokenType::RightParenthesis && !is_end()) {
                    arguments.push_back(parse_expression());

                    if (current().type != TokenType::RightParenthesis) {
                        expect(TokenType::Comma);
                    }
                }

                expect(TokenType::RightParenthesis);
                return std::make_unique<FunctionCall>(callee, std::move(arguments));
            }

            throw std::runtime_error("Expected an expression.");
        }

        std::unique_ptr<Expression> parse_expression(int min_precedence = 0) {
            auto left = parse_unary_expression();

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
                    op_token,
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
            auto value = parse_expression();

            expect(TokenType::Semicolon);

            return std::make_unique<VariableDeclaration>(
                variable_name.value,
                std::move(value),
                type
            );
        }

        std::unique_ptr<ReturnStatement> parse_return_statement() {
            expect(TokenType::Keyword, "return");

            if (current().type == TokenType::Semicolon) {
                expect(TokenType::Semicolon);
                return std::make_unique<ReturnStatement>(nullptr);
            }

            auto value = parse_expression();
            expect(TokenType::Semicolon);

            return std::make_unique<ReturnStatement>(std::move(value));
        }

        std::unique_ptr<WhileStatement> parse_while_statement() {
            expect(TokenType::Keyword, "while");
            expect(TokenType::LeftParenthesis);

            auto condition = parse_expression();
            expect(TokenType::RightParenthesis);

            auto body = parse_block();
            return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
        }

        std::unique_ptr<AssignmentStatement> parse_variable_reassignment() {
            const Token& variable = expect(TokenType::Identifier);
            expect(TokenType::Operator, "=");

            auto expression = parse_expression();
            expect(TokenType::Semicolon);

            return std::make_unique<AssignmentStatement>(variable.value, std::move(expression));
        }

        std::unique_ptr<IfStatement> parse_if_statement() {
            expect(TokenType::Keyword, "if");
            expect(TokenType::LeftParenthesis);

            auto if_condition = parse_expression();
            expect(TokenType::RightParenthesis);

            auto if_body = parse_block();
            std::unique_ptr<Statement> else_branch = nullptr;

            if (current().value == "else") {
                expect(TokenType::Keyword, "else");

                if (current().value == "if") {
                    else_branch = parse_if_statement();
                } else {
                    else_branch = parse_block();
                }
            }

            return std::make_unique<IfStatement>(std::move(if_condition), std::move(if_body), std::move(else_branch));
        }

        std::unique_ptr<BreakStatement> parse_break_statement() {
            expect(TokenType::Keyword, "break");
            expect(TokenType::Semicolon);

            return std::make_unique<BreakStatement>();
        }

        std::unique_ptr<ContinueStatement> parse_continue_statement() {
            expect(TokenType::Keyword, "continue");
            expect(TokenType::Semicolon);

            return std::make_unique<ContinueStatement>();
        }

        std::unique_ptr<Statement> parse_statement() {
            if (current().value == "let") {
                return parse_variable_declaration();
            }

            if (current().value == "return") {
                return parse_return_statement();
            }

            if (current().value == "while") {
                return parse_while_statement();
            }

            if (current().type == TokenType::Identifier && peek().value == "=") {
                return parse_variable_reassignment();
            }

            if (current().value == "if") {
                return parse_if_statement();
            }

            if (current().value == "break") {
                return parse_break_statement();
            }

            if (current().value == "continue") {
                return parse_continue_statement();
            }

            return parse_expression_statement();
        }

        std::unique_ptr<ExpressionStatement> parse_expression_statement() {
            auto expression = parse_expression();
            expect(TokenType::Semicolon);

            return std::make_unique<ExpressionStatement>(std::move(expression));
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